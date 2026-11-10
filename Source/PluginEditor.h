#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

#include "PluginProcessor.h"
#include "Theme.h"
#include "Texts.h"

//==============================================================================
//  Нарисованная клавиатура ПК: на каждой клавише подписана её нота,
//  нажатые подсвечиваются, мышью по ним тоже можно играть.
//==============================================================================
class KeyboardView : public juce::Component
{
public:
    static constexpr int keySize = 74;
    static constexpr int gap = 6;
    static constexpr int padding = 14;
    static constexpr int neededHeight = padding * 2 + KeyMap::numRows * keySize + (KeyMap::numRows - 1) * gap;

    explicit KeyboardView (KeyEngine&);
    ~KeyboardView() override;

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

    // вызывается таймером окна: перерисовать, если что-то изменилось
    void refresh();

private:
    juce::Rectangle<float> keyBounds (const KeyMap::Key&) const;
    int keyAt (juce::Point<float>) const;
    void release();
    juce::int64 pressedBits() const;
    int settingsCode() const;

    KeyEngine& engine;
    int pressedKey = -1;
    juce::int64 lastBits = -1;
    int lastSettings = -1;
};

//==============================================================================
//  Кнопка со стрелкой-треугольником
//==============================================================================
class TriButton : public juce::Button
{
public:
    enum Direction { left, right, down, up };

    explicit TriButton (Direction d) : juce::Button (juce::String()), direction (d) {}
    void paintButton (juce::Graphics&, bool isHighlighted, bool isDown) override;

private:
    Direction direction;
};

//==============================================================================
//  Значок «?» рядом с галкой
//==============================================================================
class HelpMark : public juce::Component
{
public:
    void paint (juce::Graphics&) override;
};

//==============================================================================
//  Подсказка, которая появляется сразу при наведении
//==============================================================================
class TipBox : public juce::Component
{
public:
    TipBox() { setInterceptsMouseClicks (false, false); }

    void setText (const juce::String& text, int width);
    void paint (juce::Graphics&) override;

private:
    juce::TextLayout layout;
};

//==============================================================================
//  Всё содержимое окна в «логическом» размере.
//  При растягивании окна оно целиком масштабируется.
//==============================================================================
class EditorContent : public juce::Component
{
public:
    static constexpr int logicalWidth = 1060;

    explicit EditorContent (PcKeysProcessor&);
    ~EditorContent() override;

    // высота зависит от того, скрыты ли настройки
    int logicalHeight() const;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseEnter (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;
    bool keyPressed (const juce::KeyPress&) override;

    // вызывается таймером окна
    void refresh();
    void reloadFromProcessor();
    void setHidden (bool shouldHide);

    // окно должно поменять размер (скрыли или вернули настройки)
    std::function<void()> onShapeChanged;

private:
    void applyLanguage();
    void refreshControls();
    void updateVisibility();
    void showTip (const juce::String& text, juce::Component& anchor);
    void changed();

    PcKeysProcessor& processor;
    KeyEngine& engine;
    Texts texts;

    juce::ToggleButton hideToggle, captureToggle;
    HelpMark helpMark, hideMark;
    juce::TextButton russianButton, englishButton;

    juce::Label layoutLabel, tonicLabel;
    juce::TextButton pianoButton, gridButton, scaleButton;
    juce::ComboBox tonicBox, modeBox;

    juce::Label octaveLabel, octaveValue, transposeLabel, transposeValue, velocityLabel, velocityValue;
    TriButton octaveDown { TriButton::left }, octaveUp { TriButton::right };
    TriButton transposeDown { TriButton::down }, transposeUp { TriButton::up };
    juce::Slider velocitySlider;
    juce::Label sustainLabel;
    juce::Rectangle<float> lampArea, panelArea;

    KeyboardView keyboard;

    juce::Label soundingLabel, soundingValue, hintLabel;
    TipBox tip;

    int lastArrowRevision = 0;
    juce::uint64 lastLow = 0, lastHigh = 0;
    bool lastSustain = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorContent)
};

//==============================================================================
class PcKeysEditor : public juce::AudioProcessorEditor,
                     private juce::Timer
{
public:
    explicit PcKeysEditor (PcKeysProcessor&);
    ~PcKeysEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void applyShape();

    PcKeysProcessor& processor;
    PcKeysLookAndFeel lookAndFeel;
    EditorContent content;

    int lastStateRevision = 0;
    bool ready = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PcKeysEditor)
};
