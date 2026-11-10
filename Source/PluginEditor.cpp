#include "PluginEditor.h"

#include <cmath>

namespace
{
    const juce::Colour windowColour { 0xff212121 };
    const juce::Colour sharpKeyColour { 0xff262626 };

    const char* const noteNames[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

    juce::String noteName (int note)
    {
        return juce::String (noteNames[note % 12]) + juce::String (note / 12 - 1);
    }

    bool isSharp (int note)
    {
        return noteNames[note % 12][1] != 0;
    }

    juce::String signedNumber (int value)
    {
        return (value > 0 ? "+" : "") + juce::String (value);
    }

    void setupLabel (juce::Label& label, float size, juce::Justification justification,
                     juce::Colour colour = Theme::text)
    {
        label.setFont (juce::FontOptions (size));
        label.setJustificationType (justification);
        label.setColour (juce::Label::textColourId, colour);
        label.setInterceptsMouseClicks (false, false);
        label.setMinimumHorizontalScale (1.0f);
        label.setBorderSize ({ 0, 0, 0, 0 });
    }

    // клавиши ПК должны играть ноты, а не нажимать кнопки в окне
    void noKeyboard (juce::Component& c)
    {
        c.setWantsKeyboardFocus (false);
        c.setMouseClickGrabsKeyboardFocus (false);
    }
}

//==============================================================================
//  Клавиатура
//==============================================================================
KeyboardView::KeyboardView (KeyEngine& e) : engine (e)
{
    noKeyboard (*this);
}

KeyboardView::~KeyboardView()
{
    release();
}

juce::Rectangle<float> KeyboardView::keyBounds (const KeyMap::Key& key) const
{
    const auto step = (float) (keySize + gap);

    return { (float) padding + KeyMap::rowShift[key.row] * step + (float) key.col * step,
             (float) padding + (float) key.row * step,
             (float) keySize, (float) keySize };
}

int KeyboardView::keyAt (juce::Point<float> position) const
{
    const auto& keys = KeyMap::keys();

    for (size_t i = 0; i < keys.size(); ++i)
        if (keyBounds (keys[i]).contains (position))
            return (int) i;

    return -1;
}

juce::int64 KeyboardView::pressedBits() const
{
    juce::int64 bits = 0;

    for (size_t i = 0; i < engine.keyOn.size(); ++i)
        if (engine.keyOn[i].load())
            bits |= (juce::int64) 1 << i;

    return bits;
}

int KeyboardView::settingsCode() const
{
    int code = engine.layout.load();
    code = code * 16 + engine.tonic.load();
    code = code * 16 + engine.mode.load();
    code = code * 16 + (engine.octave.load() + 8);
    code = code * 32 + (engine.transpose.load() + 16);
    return code;
}

void KeyboardView::refresh()
{
    const auto bits = pressedBits();
    const auto settings = settingsCode();

    if (bits != lastBits || settings != lastSettings)
    {
        lastBits = bits;
        lastSettings = settings;
        repaint();
    }
}

void KeyboardView::paint (juce::Graphics& g)
{
    const auto area = getLocalBounds().toFloat().reduced (0.5f);

    g.setColour (Theme::background);
    g.fillRoundedRectangle (area, 8.0f);
    g.setColour (Theme::panelBorder);
    g.drawRoundedRectangle (area, 8.0f, 1.0f);

    const int layout = engine.layout.load();
    const int tonic = engine.tonic.load();
    const int mode = engine.mode.load();
    const int octave = engine.octave.load();
    const int transpose = engine.transpose.load();

    const auto& keys = KeyMap::keys();

    for (size_t i = 0; i < keys.size(); ++i)
    {
        const auto& key = keys[i];
        const auto bounds = keyBounds (key);
        const int note = KeyMap::noteFor (key, layout, tonic, mode, octave, transpose);
        const bool on = engine.keyOn[i].load();
        const auto capArea = juce::Rectangle<float> (bounds.getX() + 7.0f, bounds.getY() + 4.0f, 24.0f, 18.0f);
        const juce::String cap = juce::String::charToString ((juce::juce_wchar) key.cap);

        if (note < 0)
        {
            // свободная клавиша
            g.setColour (Theme::panelBorder);
            g.drawRoundedRectangle (bounds.reduced (0.5f), 6.0f, 1.0f);
            g.setColour (Theme::textHint);
            g.setFont (juce::FontOptions (13.5f));
            g.drawText (cap, capArea, juce::Justification::topLeft, false);
            continue;
        }

        const bool sharp = isSharp (note);

        g.setColour (on ? Theme::tonic : (sharp ? sharpKeyColour : Theme::button));
        g.fillRoundedRectangle (bounds, 6.0f);

        // тоника в раскладке «Только ноты лада» обведена
        if (! on && KeyMap::isRoot (key, layout))
        {
            g.setColour (Theme::tonic);
            g.drawRoundedRectangle (bounds.reduced (1.0f), 6.0f, 2.0f);
        }

        g.setColour (on ? juce::Colours::white : Theme::textMuted);
        g.setFont (juce::FontOptions (13.5f));
        g.drawText (cap, capArea, juce::Justification::topLeft, false);

        g.setColour (on ? juce::Colours::white : (sharp ? Theme::textMuted : Theme::text));
        g.setFont (juce::FontOptions (20.0f, juce::Font::bold));
        g.drawText (noteName (note), bounds, juce::Justification::centred, false);
    }
}

void KeyboardView::release()
{
    if (pressedKey >= 0)
        engine.mouseDown[(size_t) pressedKey].store (false);

    pressedKey = -1;
}

void KeyboardView::mouseDown (const juce::MouseEvent& e)
{
    release();
    pressedKey = keyAt (e.position);

    if (pressedKey >= 0)
        engine.mouseDown[(size_t) pressedKey].store (true);
}

void KeyboardView::mouseDrag (const juce::MouseEvent& e)
{
    // увели мышь с клавиши — нота снимается
    if (pressedKey >= 0 && keyAt (e.position) != pressedKey)
        release();
}

void KeyboardView::mouseUp (const juce::MouseEvent&)
{
    release();
}

//==============================================================================
void TriButton::paintButton (juce::Graphics& g, bool isHighlighted, bool isDown)
{
    const auto bounds = getLocalBounds().toFloat().reduced (0.5f);

    g.setColour (isDown ? Theme::buttonActive : (isHighlighted ? Theme::buttonHover : Theme::button));
    g.fillRoundedRectangle (bounds, 6.0f);

    const auto c = bounds.getCentre();
    const float s = 5.0f;
    juce::Path p;

    switch (direction)
    {
        case left:  p.addTriangle (c.x - s, c.y, c.x + s, c.y - s, c.x + s, c.y + s); break;
        case right: p.addTriangle (c.x + s, c.y, c.x - s, c.y - s, c.x - s, c.y + s); break;
        case down:  p.addTriangle (c.x, c.y + s, c.x - s, c.y - s, c.x + s, c.y - s); break;
        case up:    p.addTriangle (c.x, c.y - s, c.x - s, c.y + s, c.x + s, c.y + s); break;
    }

    g.setColour (Theme::text);
    g.fillPath (p);
}

void HelpMark::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    g.setColour (Theme::button);
    g.fillEllipse (bounds);
    g.setColour (Theme::textMuted);
    g.setFont (juce::FontOptions (12.5f));
    g.drawText ("?", bounds, juce::Justification::centred, false);
}

void TipBox::setText (const juce::String& text, int width)
{
    juce::AttributedString attributed;
    attributed.setWordWrap (juce::AttributedString::byWord);
    attributed.append (text, juce::FontOptions (13.5f), Theme::text);

    layout.createLayout (attributed, (float) (width - 22));
    setSize (width, (int) std::ceil (layout.getHeight()) + 16);
    repaint();
}

void TipBox::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat().reduced (0.5f);

    g.setColour (Theme::background);
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setColour (Theme::buttonActive);
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

    layout.draw (g, getLocalBounds().reduced (11, 8).toFloat());
}

//==============================================================================
//  Содержимое окна
//==============================================================================
EditorContent::EditorContent (PcKeysProcessor& p)
    : processor (p), engine (p.engine), keyboard (p.engine)
{
    setWantsKeyboardFocus (true);

    // --- верхняя строка: «Скрыть все настройки» и язык
    addAndMakeVisible (hideToggle);
    hideToggle.onClick = [this] { setHidden (hideToggle.getToggleState()); };
    addAndMakeVisible (hideMark);

    for (auto* b : { &russianButton, &englishButton })
    {
        addAndMakeVisible (*b);
        b->setClickingTogglesState (false);
        b->setRadioGroupId (1001);
    }

    russianButton.setButtonText ("RU");
    englishButton.setButtonText ("EN");
    russianButton.onClick = [this] { processor.setLanguage (PcKeysProcessor::russian); applyLanguage(); };
    englishButton.onClick = [this] { processor.setLanguage (PcKeysProcessor::english); applyLanguage(); };

    // --- раскладка
    addAndMakeVisible (layoutLabel);
    setupLabel (layoutLabel, 14.0f, juce::Justification::centredLeft, Theme::textMuted);

    int layoutIndex = 0;

    for (auto* b : { &pianoButton, &gridButton, &scaleButton })
    {
        addAndMakeVisible (*b);
        b->setClickingTogglesState (false);
        b->setRadioGroupId (1002);

        const int index = layoutIndex++;
        b->onClick = [this, index]
        {
            engine.layout = index;
            changed();
            updateVisibility();
        };
    }

    addAndMakeVisible (captureToggle);
    captureToggle.onClick = [this]
    {
        engine.captureOutside = captureToggle.getToggleState();
        changed();
    };

    addAndMakeVisible (helpMark);

    addAndMakeVisible (tonicLabel);
    setupLabel (tonicLabel, 14.0f, juce::Justification::centredLeft, Theme::textMuted);

    addAndMakeVisible (tonicBox);
    for (int i = 0; i < 12; ++i)
        tonicBox.addItem (noteNames[i], i + 1);

    tonicBox.onChange = [this]
    {
        engine.tonic = juce::jlimit (0, 11, tonicBox.getSelectedId() - 1);
        changed();
    };

    addAndMakeVisible (modeBox);
    modeBox.onChange = [this]
    {
        engine.mode = juce::jlimit (0, KeyMap::numModes - 1, modeBox.getSelectedId() - 1);
        changed();
    };

    // --- октава, полутоны, velocity, сустейн
    for (auto* l : { &octaveLabel, &transposeLabel, &velocityLabel })
    {
        addAndMakeVisible (*l);
        setupLabel (*l, 14.0f, juce::Justification::centredLeft, Theme::textMuted);

        // нажатие на название возвращает значение по умолчанию
        l->setInterceptsMouseClicks (true, false);
        l->setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    for (auto* l : { &octaveValue, &transposeValue, &velocityValue })
    {
        addAndMakeVisible (*l);
        setupLabel (*l, 14.0f, juce::Justification::centred);
    }

    for (auto* b : { &octaveDown, &octaveUp, &transposeDown, &transposeUp })
        addAndMakeVisible (*b);

    octaveDown.onClick    = [this] { engine.octave = juce::jmax (-KeyEngine::maxOctave, engine.octave.load() - 1); changed(); };
    octaveUp.onClick      = [this] { engine.octave = juce::jmin (KeyEngine::maxOctave, engine.octave.load() + 1); changed(); };
    transposeDown.onClick = [this] { engine.transpose = juce::jmax (-KeyEngine::maxTranspose, engine.transpose.load() - 1); changed(); };
    transposeUp.onClick   = [this] { engine.transpose = juce::jmin (KeyEngine::maxTranspose, engine.transpose.load() + 1); changed(); };

    addAndMakeVisible (velocitySlider);
    velocitySlider.setSliderStyle (juce::Slider::LinearHorizontal);
    velocitySlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    velocitySlider.setRange (1.0, 127.0, 1.0);
    velocitySlider.setColour (juce::Slider::trackColourId, Theme::tonic);
    velocitySlider.setColour (juce::Slider::backgroundColourId, Theme::button);
    velocitySlider.setColour (juce::Slider::thumbColourId, Theme::text);
    velocitySlider.onValueChange = [this]
    {
        engine.velocity = juce::roundToInt (velocitySlider.getValue());
        velocityValue.setText (juce::String (engine.velocity.load()), juce::dontSendNotification);
    };
    velocitySlider.onDragEnd = [this] { processor.notifyHostAboutChange(); };

    addAndMakeVisible (sustainLabel);
    setupLabel (sustainLabel, 14.0f, juce::Justification::centredLeft, Theme::textMuted);

    // --- клавиатура и строка под ней
    addAndMakeVisible (keyboard);

    addAndMakeVisible (soundingLabel);
    setupLabel (soundingLabel, 14.0f, juce::Justification::centredLeft, Theme::textMuted);
    addAndMakeVisible (soundingValue);
    setupLabel (soundingValue, 14.0f, juce::Justification::centredLeft);
    addAndMakeVisible (hintLabel);
    setupLabel (hintLabel, 14.0f, juce::Justification::centredRight, Theme::textMuted);

    addChildComponent (tip);

    // события мыши от всех элементов окна: подсказки при наведении, сброс по нажатию на название
    addMouseListener (this, true);

    for (auto* c : std::initializer_list<juce::Component*> { &hideToggle, &hideMark, &captureToggle, &helpMark,
                                                              &russianButton, &englishButton,
                                                              &pianoButton, &gridButton, &scaleButton,
                                                              &tonicBox, &modeBox,
                                                              &octaveDown, &octaveUp, &transposeDown, &transposeUp,
                                                              &velocitySlider,
                                                              &octaveLabel, &transposeLabel, &velocityLabel })
        noKeyboard (*c);

    lastArrowRevision = engine.arrowRevision.load();

    applyLanguage();
    updateVisibility();
}

EditorContent::~EditorContent()
{
    removeMouseListener (this);
}

int EditorContent::logicalHeight() const
{
    return processor.getHideSettings() ? 12 + KeyboardView::neededHeight + 12
                                       : 156 + KeyboardView::neededHeight + 44;
}

void EditorContent::changed()
{
    processor.notifyHostAboutChange();
    refreshControls();
}

void EditorContent::applyLanguage()
{
    texts = Texts::get (processor.getLanguage());

    hideToggle.setButtonText (texts.hideAll);
    captureToggle.setButtonText (texts.capture);
    layoutLabel.setText (texts.layout, juce::dontSendNotification);
    pianoButton.setButtonText (texts.piano);
    gridButton.setButtonText (texts.grid);
    scaleButton.setButtonText (texts.scale);
    tonicLabel.setText (texts.tonic, juce::dontSendNotification);
    octaveLabel.setText (texts.octave, juce::dontSendNotification);
    transposeLabel.setText (texts.transpose, juce::dontSendNotification);
    velocityLabel.setText (texts.velocity, juce::dontSendNotification);
    sustainLabel.setText (texts.sustain, juce::dontSendNotification);
    soundingLabel.setText (texts.sounding, juce::dontSendNotification);
    hintLabel.setText (texts.hint, juce::dontSendNotification);

    modeBox.clear (juce::dontSendNotification);
    for (int i = 0; i < texts.modes.size(); ++i)
        modeBox.addItem (texts.modes[i], i + 1);

    tip.setVisible (false);
    refreshControls();
    resized();
}

void EditorContent::refreshControls()
{
    const int language = processor.getLanguage();
    russianButton.setToggleState (language == PcKeysProcessor::russian, juce::dontSendNotification);
    englishButton.setToggleState (language == PcKeysProcessor::english, juce::dontSendNotification);

    const int layout = engine.layout.load();
    pianoButton.setToggleState (layout == KeyMap::piano, juce::dontSendNotification);
    gridButton.setToggleState (layout == KeyMap::grid, juce::dontSendNotification);
    scaleButton.setToggleState (layout == KeyMap::scale, juce::dontSendNotification);

    hideToggle.setToggleState (processor.getHideSettings(), juce::dontSendNotification);
    captureToggle.setToggleState (engine.captureOutside.load(), juce::dontSendNotification);

    tonicBox.setSelectedId (engine.tonic.load() + 1, juce::dontSendNotification);
    modeBox.setSelectedId (engine.mode.load() + 1, juce::dontSendNotification);

    octaveValue.setText (signedNumber (engine.octave.load()), juce::dontSendNotification);
    transposeValue.setText (signedNumber (engine.transpose.load()), juce::dontSendNotification);
    velocityValue.setText (juce::String (engine.velocity.load()), juce::dontSendNotification);
    velocitySlider.setValue ((double) engine.velocity.load(), juce::dontSendNotification);
}

void EditorContent::updateVisibility()
{
    const bool shown = ! processor.getHideSettings();
    const bool scaleLayout = engine.layout.load() == KeyMap::scale;

    for (auto* c : std::initializer_list<juce::Component*> { &hideToggle, &hideMark, &russianButton, &englishButton,
                                                              &layoutLabel, &pianoButton, &gridButton, &scaleButton,
                                                              &captureToggle, &helpMark,
                                                              &octaveLabel, &octaveDown, &octaveValue, &octaveUp,
                                                              &transposeLabel, &transposeDown, &transposeValue, &transposeUp,
                                                              &velocityLabel, &velocitySlider, &velocityValue, &sustainLabel,
                                                              &soundingLabel, &soundingValue, &hintLabel })
        c->setVisible (shown);

    for (auto* c : std::initializer_list<juce::Component*> { &tonicLabel, &tonicBox, &modeBox })
        c->setVisible (shown && scaleLayout);

    tip.setVisible (false);
    repaint();
}

void EditorContent::setHidden (bool shouldHide)
{
    processor.setHideSettings (shouldHide);
    refreshControls();
    updateVisibility();
    resized();

    if (onShapeChanged != nullptr)
        onShapeChanged();
}

void EditorContent::reloadFromProcessor()
{
    applyLanguage();
    updateVisibility();

    if (onShapeChanged != nullptr)
        onShapeChanged();
}

//==============================================================================
void EditorContent::paint (juce::Graphics& g)
{
    g.fillAll (windowColour);

    if (processor.getHideSettings())
        return;

    g.setColour (Theme::panel);
    g.fillRoundedRectangle (panelArea, 8.0f);
    g.setColour (Theme::panelBorder);
    g.drawRoundedRectangle (panelArea, 8.0f, 1.0f);

    // лампочка сустейна
    g.setColour (engine.sustainOn.load() ? Theme::handle : Theme::button);
    g.fillEllipse (lampArea);
}

void EditorContent::resized()
{
    const int margin = 16;
    const int keyboardWidth = logicalWidth - margin * 2;

    if (processor.getHideSettings())
    {
        keyboard.setBounds (margin, 12, keyboardWidth, KeyboardView::neededHeight);
        return;
    }

    // верхняя строка
    englishButton.setBounds (logicalWidth - margin - 44, 12, 44, 28);
    russianButton.setBounds (englishButton.getX() - 46, 12, 44, 28);
    hideMark.setBounds (russianButton.getX() - 16 - 18, 17, 18, 18);
    hideToggle.setBounds (hideMark.getX() - 4 - 180, 12, 180, 28);

    // панель настроек
    panelArea = juce::Rectangle<int> (margin, 50, keyboardWidth, 94).toFloat().reduced (0.5f);

    int x = margin + 12;
    int y = 62;
    const int h = 30;

    layoutLabel.setBounds (x, y, 76, h);            x += 82;
    pianoButton.setBounds (x, y, 86, h);            x += 88;
    gridButton.setBounds (x, y, 72, h);             x += 74;
    scaleButton.setBounds (x, y, 150, h);           x += 150 + 20;
    captureToggle.setBounds (x, y, 222, h);         x += 222 + 4;
    helpMark.setBounds (x, y + 6, 18, 18);          x += 18 + 20;
    tonicLabel.setBounds (x, y, 54, h);             x += 60;
    tonicBox.setBounds (x, y, 66, h);               x += 72;
    modeBox.setBounds (x, y, 192, h);

    x = margin + 12;
    y += h + 10;

    octaveLabel.setBounds (x, y, 62, h);            x += 66;
    octaveDown.setBounds (x, y, 30, h);             x += 30;
    octaveValue.setBounds (x, y, 44, h);            x += 44;
    octaveUp.setBounds (x, y, 30, h);               x += 30 + 26;

    transposeLabel.setBounds (x, y, 78, h);         x += 82;
    transposeDown.setBounds (x, y, 30, h);          x += 30;
    transposeValue.setBounds (x, y, 44, h);         x += 44;
    transposeUp.setBounds (x, y, 30, h);            x += 30 + 26;

    velocityLabel.setBounds (x, y, 62, h);          x += 66;
    velocitySlider.setBounds (x, y, 200, h);        x += 200;
    velocityValue.setBounds (x, y, 44, h);          x += 44 + 26;

    lampArea = juce::Rectangle<float> ((float) x, (float) y + 9.0f, 12.0f, 12.0f);
    x += 20;
    sustainLabel.setBounds (x, y, 170, h);

    // клавиатура и строка под ней
    keyboard.setBounds (margin, 156, keyboardWidth, KeyboardView::neededHeight);

    const int statusY = keyboard.getBottom() + 10;
    soundingLabel.setBounds (margin, statusY, 76, 22);
    soundingValue.setBounds (margin + 78, statusY, 330, 22);
    hintLabel.setBounds (margin + 410, statusY, keyboardWidth - 410, 22);
}

//==============================================================================
void EditorContent::showTip (const juce::String& text, juce::Component& anchor)
{
    tip.setText (text, 340);

    const auto area = getLocalArea (&anchor, anchor.getLocalBounds());
    const int x = juce::jlimit (8, logicalWidth - tip.getWidth() - 8, area.getX());

    tip.setTopLeftPosition (x, area.getBottom() + 6);
    tip.setVisible (true);
    tip.toFront (false);
}

void EditorContent::mouseEnter (const juce::MouseEvent& e)
{
    auto* c = e.eventComponent;

    if (c == &captureToggle || c == &helpMark)
        showTip (texts.captureTip, *c);
    else if (c == &hideToggle || c == &hideMark)
        showTip (texts.hideTip, *c);
    else if (c == &octaveLabel || c == &transposeLabel || c == &velocityLabel)
        showTip (texts.resetTip, *c);
}

void EditorContent::mouseExit (const juce::MouseEvent& e)
{
    if (e.eventComponent != this)
        tip.setVisible (false);
}

void EditorContent::mouseDown (const juce::MouseEvent&)
{
    // после любого щелчка в окне клавиши ПК идут в плагин
    grabKeyboardFocus();
}

void EditorContent::mouseUp (const juce::MouseEvent& e)
{
    auto* c = e.eventComponent;

    if (c == this || c == nullptr || ! c->getLocalBounds().contains (e.getPosition()))
        return;

    // нажатие на название — значение по умолчанию
    if (c == &octaveLabel)          engine.octave = KeyEngine::defaultOctave;
    else if (c == &transposeLabel)  engine.transpose = KeyEngine::defaultTranspose;
    else if (c == &velocityLabel)   engine.velocity = KeyEngine::defaultVelocity;
    else return;

    changed();
}

bool EditorContent::keyPressed (const juce::KeyPress& key)
{
    // клавиши забирает плагин, чтобы они не уходили в DAW; сочетания с Ctrl и Alt не трогаем
    const auto mods = key.getModifiers();
    return ! (mods.isCtrlDown() || mods.isAltDown() || mods.isCommandDown());
}

void EditorContent::refresh()
{
    const int revision = engine.arrowRevision.load();

    // октаву или полутоны поменяли стрелками
    if (revision != lastArrowRevision)
    {
        lastArrowRevision = revision;
        changed();
    }

    const bool sustain = engine.sustainOn.load();

    if (sustain != lastSustain)
    {
        lastSustain = sustain;
        repaint (lampArea.getSmallestIntegerContainer().expanded (2));
    }

    const auto low = engine.soundingLow.load();
    const auto high = engine.soundingHigh.load();

    if (low != lastLow || high != lastHigh)
    {
        lastLow = low;
        lastHigh = high;

        juce::StringArray names;

        for (int n = 0; n < 128; ++n)
        {
            const bool on = n < 64 ? ((low >> n) & 1) != 0 : ((high >> (n - 64)) & 1) != 0;

            if (on)
                names.add (noteName (n));
        }

        soundingValue.setText (names.joinIntoString ("  "), juce::dontSendNotification);
    }

    keyboard.refresh();
}

//==============================================================================
//  Окно плагина
//==============================================================================
PcKeysEditor::PcKeysEditor (PcKeysProcessor& p)
    : juce::AudioProcessorEditor (&p),
      processor (p),
      content (p)
{
    setLookAndFeel (&lookAndFeel);
    addAndMakeVisible (content);
    setWantsKeyboardFocus (true);

    content.onShapeChanged = [this] { applyShape(); };

    // окно тянется за правый нижний угол, пропорции всегда сохраняются
    setResizable (true, true);
    applyShape();

    lastStateRevision = processor.getStateRevision();
    ready = true;

    startTimerHz (60);
}

PcKeysEditor::~PcKeysEditor()
{
    stopTimer();
    processor.engine.editorWindow = nullptr;
    setLookAndFeel (nullptr);
}

void PcKeysEditor::applyShape()
{
    const int width = EditorContent::logicalWidth;
    const int height = content.logicalHeight();
    const auto scale = juce::jlimit (0.5, 2.5, processor.editorScale);

    setResizeLimits (width / 2, height / 2, width * 5 / 2, height * 5 / 2);

    if (auto* sizeLimits = getConstrainer())
        sizeLimits->setFixedAspectRatio ((double) width / height);

    setSize (juce::roundToInt (width * scale), juce::roundToInt (height * scale));
    resized();
}

void PcKeysEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff212121));
}

void PcKeysEditor::resized()
{
    const auto scale = (float) getWidth() / (float) EditorContent::logicalWidth;

    content.setBounds (0, 0, EditorContent::logicalWidth, content.logicalHeight());
    content.setTransform (juce::AffineTransform::scale (scale));

    if (ready)
        processor.editorScale = scale;
}

void PcKeysEditor::timerCallback()
{
    // по окну плагина определяется, активно ли оно
    auto* peer = getPeer();
    processor.engine.editorWindow = peer != nullptr ? peer->getNativeHandle() : nullptr;

    // Esc возвращает скрытые настройки
    if (processor.engine.escapePressed.exchange (false) && processor.getHideSettings())
        content.setHidden (false);

    const auto revision = processor.getStateRevision();

    // DAW загрузила проект уже после открытия окна
    if (revision != lastStateRevision)
    {
        lastStateRevision = revision;
        content.reloadFromProcessor();
    }

    content.refresh();
}
