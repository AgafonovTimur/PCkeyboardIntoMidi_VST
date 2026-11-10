#pragma once

#include <juce_core/juce_core.h>
#include <array>
#include <atomic>
#include <vector>

#include "KeyMap.h"

//==============================================================================
//  Следит за клавишами ПК и мышью и превращает нажатия в MIDI-ноты.
//  Работает в своём потоке, проверяет клавиши тысячу раз в секунду.
//==============================================================================
class KeyEngine : private juce::Thread
{
public:
    static constexpr int defaultOctave = 0;
    static constexpr int defaultTranspose = 0;
    static constexpr int defaultVelocity = 70;
    static constexpr int maxOctave = 3;
    static constexpr int maxTranspose = 11;

    KeyEngine();
    ~KeyEngine() override;

    // настройки (меняет окно плагина; октаву и полутоны — ещё и стрелки)
    std::atomic<int> layout { KeyMap::piano }, tonic { 0 }, mode { 0 };
    std::atomic<int> octave { defaultOctave }, transpose { defaultTranspose }, velocity { defaultVelocity };
    std::atomic<bool> captureOutside { false };

    // окно плагина (нужно, чтобы понять, активно ли оно) и клавиши, зажатые мышью
    std::atomic<void*> editorWindow { nullptr };
    std::array<std::atomic<bool>, KeyMap::numKeys> mouseDown;

    // для показа в окне
    std::array<std::atomic<bool>, KeyMap::numKeys> keyOn;
    std::atomic<juce::uint64> soundingLow { 0 }, soundingHigh { 0 };
    std::atomic<bool> sustainOn { false };
    std::atomic<bool> escapePressed { false };
    std::atomic<int> arrowRevision { 0 };      // растёт, когда стрелки поменяли октаву или полутоны

    // готовые MIDI-сообщения забирает поток звука
    bool popMidi (std::array<juce::uint8, 3>& message);

private:
    void run() override;
    void tick();

    void send (juce::uint8 status, int note, int value);
    void noteOn (int note);
    void noteOff (int note);
    void publishSounding();

    // очередь: пишет только этот поток, читает только поток звука
    static constexpr int capacity = 1024;
    juce::AbstractFifo fifo { capacity };
    std::vector<std::array<juce::uint8, 3>> storage;

    std::array<unsigned, KeyMap::numKeys> scanCodes {}, defaultCodes {}, keyCodes {};
    void* lastKeyboardLayout = nullptr;

    std::array<bool, KeyMap::numKeys> isDown {};
    std::array<int, KeyMap::numKeys> heldNote {};
    std::array<int, 128> noteCount {};
    std::array<bool, 128> sustained {};
    std::array<bool, 4> arrowDown {};
    bool escapeDown = false;
    bool sustain = false;
    int lastLayout = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyEngine)
};
