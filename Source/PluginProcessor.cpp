#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PcKeysProcessor::PcKeysProcessor()
    : AudioProcessor (BusesProperties()
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // язык по умолчанию — как в системе: русский или английский
    const auto systemLanguage = juce::SystemStats::getUserLanguage().toLowerCase();
    language = systemLanguage.startsWith ("ru") ? russian : english;
}

bool PcKeysProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();

    return out == juce::AudioChannelSet::stereo()
        || out == juce::AudioChannelSet::mono();
}

//==============================================================================
void PcKeysProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                    juce::MidiBuffer& midiMessages)
{
    // плагин сам ничего не озвучивает
    buffer.clear();

    // ноты с клавиатуры ПК добавляются к тому, что пришло по MIDI
    std::array<juce::uint8, 3> message {};

    while (engine.popMidi (message))
        midiMessages.addEvent (message.data(), 3, 0);
}

//==============================================================================
void PcKeysProcessor::setLanguage (int newLanguage)
{
    language = (newLanguage == english) ? english : russian;
    notifyHostAboutChange();
}

void PcKeysProcessor::setHideSettings (bool shouldHide)
{
    hideSettings = shouldHide;
    notifyHostAboutChange();
}

void PcKeysProcessor::notifyHostAboutChange()
{
    updateHostDisplay (juce::AudioProcessorListener::ChangeDetails {}
                           .withNonParameterStateChanged (true));
}

//==============================================================================
juce::AudioProcessorEditor* PcKeysProcessor::createEditor()
{
    return new PcKeysEditor (*this);
}

void PcKeysProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ValueTree state ("PCkeyboardIntoMidiState");

    state.setProperty ("layout", engine.layout.load(), nullptr);
    state.setProperty ("tonic", engine.tonic.load(), nullptr);
    state.setProperty ("mode", engine.mode.load(), nullptr);
    state.setProperty ("octave", engine.octave.load(), nullptr);
    state.setProperty ("transpose", engine.transpose.load(), nullptr);
    state.setProperty ("velocity", engine.velocity.load(), nullptr);
    state.setProperty ("captureOutside", engine.captureOutside.load(), nullptr);
    state.setProperty ("hideSettings", hideSettings.load(), nullptr);
    state.setProperty ("language", getLanguage(), nullptr);
    state.setProperty ("scale", editorScale, nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void PcKeysProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        const auto state = juce::ValueTree::fromXml (*xml);

        if (! state.isValid())
            return;

        engine.layout    = juce::jlimit (0, 2, (int) state.getProperty ("layout", 0));
        engine.tonic     = juce::jlimit (0, 11, (int) state.getProperty ("tonic", 0));
        engine.mode      = juce::jlimit (0, KeyMap::numModes - 1, (int) state.getProperty ("mode", 0));
        engine.octave    = juce::jlimit (-KeyEngine::maxOctave, KeyEngine::maxOctave,
                                         (int) state.getProperty ("octave", KeyEngine::defaultOctave));
        engine.transpose = juce::jlimit (-KeyEngine::maxTranspose, KeyEngine::maxTranspose,
                                         (int) state.getProperty ("transpose", KeyEngine::defaultTranspose));
        engine.velocity  = juce::jlimit (1, 127, (int) state.getProperty ("velocity", KeyEngine::defaultVelocity));
        engine.captureOutside = (bool) state.getProperty ("captureOutside", false);

        hideSettings = (bool) state.getProperty ("hideSettings", false);
        language = ((int) state.getProperty ("language", (int) language.load()) == english) ? english : russian;
        editorScale = juce::jlimit (0.5, 2.5, (double) state.getProperty ("scale", editorScale));

        // окно плагина могло быть открыто раньше, чем DAW прислала настройки
        stateRevision.fetch_add (1);
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PcKeysProcessor();
}
