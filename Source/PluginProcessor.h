#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>

#include "KeyEngine.h"

//==============================================================================
//  PCkeyboardIntoMidi: превращает клавиатуру ПК в MIDI-клавиатуру.
//  Ноты уходят в MIDI-выход плагина; пришедшие MIDI-сообщения проходят без изменений.
//==============================================================================
class PcKeysProcessor : public juce::AudioProcessor
{
public:
    enum Language { russian = 0, english = 1 };

    PcKeysProcessor();
    ~PcKeysProcessor() override = default;

    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==========================================================================
    KeyEngine engine;

    int  getLanguage() const noexcept { return language.load(); }
    void setLanguage (int newLanguage);

    bool getHideSettings() const noexcept { return hideSettings.load(); }
    void setHideSettings (bool shouldHide);

    // сообщить DAW, что проект изменился и его надо сохранить
    void notifyHostAboutChange();

    // растёт, когда DAW загрузила настройки из проекта
    int getStateRevision() const noexcept { return stateRevision.load(); }

    // масштаб окна плагина — сохраняется вместе с проектом
    double editorScale = 0.85;

private:
    std::atomic<int> language { russian };
    std::atomic<bool> hideSettings { false };
    std::atomic<int> stateRevision { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PcKeysProcessor)
};
