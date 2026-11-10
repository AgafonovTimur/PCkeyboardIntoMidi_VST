#include "KeyEngine.h"
#include "WinKeys.h"

KeyEngine::KeyEngine() : juce::Thread ("PCkeyboardIntoMidi keys")
{
    storage.resize ((size_t) capacity);

    const auto& keys = KeyMap::keys();

    for (size_t i = 0; i < keys.size(); ++i)
    {
        scanCodes[i] = keys[i].scanCode;
        defaultCodes[i] = keys[i].defaultVk;
        keyCodes[i] = keys[i].defaultVk;
        mouseDown[i].store (false);
        keyOn[i].store (false);
        heldNote[i] = -1;
    }

    startThread();
}

KeyEngine::~KeyEngine()
{
    stopThread (1000);
}

bool KeyEngine::popMidi (std::array<juce::uint8, 3>& message)
{
    const auto scope = fifo.read (1);

    if (scope.blockSize1 > 0) { message = storage[(size_t) scope.startIndex1]; return true; }
    if (scope.blockSize2 > 0) { message = storage[(size_t) scope.startIndex2]; return true; }

    return false;
}

void KeyEngine::send (juce::uint8 status, int note, int value)
{
    const auto scope = fifo.write (1);
    const std::array<juce::uint8, 3> message { status, (juce::uint8) note, (juce::uint8) value };

    if (scope.blockSize1 > 0)       storage[(size_t) scope.startIndex1] = message;
    else if (scope.blockSize2 > 0)  storage[(size_t) scope.startIndex2] = message;
}

void KeyEngine::noteOn (int note)
{
    const auto n = (size_t) note;

    // нота уже звучит (с другой клавиши или на сустейне) — берём её заново
    if (noteCount[n] > 0 || sustained[n])
        send (0x80, note, 0);

    ++noteCount[n];
    sustained[n] = false;
    send (0x90, note, juce::jlimit (1, 127, velocity.load()));
}

void KeyEngine::noteOff (int note)
{
    const auto n = (size_t) note;

    if (noteCount[n] <= 0)
        return;

    if (--noteCount[n] > 0)
        return;

    if (sustain)
        sustained[n] = true;
    else
        send (0x80, note, 0);
}

void KeyEngine::publishSounding()
{
    juce::uint64 low = 0, high = 0;

    for (int n = 0; n < 128; ++n)
    {
        if (noteCount[(size_t) n] > 0 || sustained[(size_t) n])
        {
            if (n < 64) low  |= (juce::uint64) 1 << n;
            else        high |= (juce::uint64) 1 << (n - 64);
        }
    }

    soundingLow.store (low);
    soundingHigh.store (high);
}

void KeyEngine::run()
{
    WinKeys::beginFineTimer();

    while (! threadShouldExit())
    {
        tick();
        wait (1);
    }

    WinKeys::endFineTimer();
}

void KeyEngine::tick()
{
    const bool windowActive = WinKeys::isWindowActive (editorWindow.load());
    const bool modifier = WinKeys::isModifierDown();
    const bool listen = (windowActive || captureOutside.load()) && ! modifier;

    WinKeys::updateKeyCodes (lastKeyboardLayout, scanCodes.data(), defaultCodes.data(),
                             keyCodes.data(), KeyMap::numKeys);

    // стрелки и Esc — только пока активно окно плагина
    const WinKeys::Special arrows[4] = { WinKeys::left, WinKeys::right, WinKeys::down, WinKeys::up };

    for (size_t i = 0; i < 4; ++i)
    {
        const bool now = windowActive && ! modifier && WinKeys::isSpecialDown (arrows[i]);

        if (now && ! arrowDown[i])
        {
            if (i == 0) octave = juce::jlimit (-maxOctave, maxOctave, octave.load() - 1);
            if (i == 1) octave = juce::jlimit (-maxOctave, maxOctave, octave.load() + 1);
            if (i == 2) transpose = juce::jlimit (-maxTranspose, maxTranspose, transpose.load() - 1);
            if (i == 3) transpose = juce::jlimit (-maxTranspose, maxTranspose, transpose.load() + 1);
            arrowRevision.fetch_add (1);
        }

        arrowDown[i] = now;
    }

    {
        const bool now = windowActive && WinKeys::isSpecialDown (WinKeys::escape);

        if (now && ! escapeDown)
            escapePressed = true;

        escapeDown = now;
    }

    // сменилась раскладка нот — снимаем всё, что звучало
    const int currentLayout = layout.load();
    bool changed = false;

    if (currentLayout != lastLayout)
    {
        for (size_t i = 0; i < isDown.size(); ++i)
        {
            if (heldNote[i] >= 0)
            {
                noteOff (heldNote[i]);
                heldNote[i] = -1;       // клавиша заиграет снова после того, как её отпустят и нажмут
                changed = true;
            }
        }

        lastLayout = currentLayout;
    }

    // сустейн — пробел
    const bool sustainNow = listen && WinKeys::isSpecialDown (WinKeys::space);

    if (sustainNow != sustain)
    {
        sustain = sustainNow;
        sustainOn = sustain;
        changed = true;

        if (! sustain)
        {
            for (int n = 0; n < 128; ++n)
            {
                if (sustained[(size_t) n])
                {
                    sustained[(size_t) n] = false;
                    send (0x80, n, 0);
                }
            }
        }
    }

    // клавиши с нотами
    const auto& keys = KeyMap::keys();

    for (size_t i = 0; i < keys.size(); ++i)
    {
        const bool now = (listen && WinKeys::isKeyDown (keyCodes[i])) || mouseDown[i].load();

        if (now && ! isDown[i])
        {
            isDown[i] = true;
            heldNote[i] = KeyMap::noteFor (keys[i], currentLayout, tonic.load(), mode.load(),
                                           octave.load(), transpose.load());
            if (heldNote[i] >= 0)
                noteOn (heldNote[i]);

            changed = true;
        }
        else if (! now && isDown[i])
        {
            isDown[i] = false;

            if (heldNote[i] >= 0)
                noteOff (heldNote[i]);

            heldNote[i] = -1;
            changed = true;
        }

        keyOn[i].store (isDown[i] && heldNote[i] >= 0);
    }

    if (changed)
        publishSounding();
}
