#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
//  Цвета — тёмная тема из Ear Trainer (панель настроек, тема «Тёмная»)
//==============================================================================
namespace Theme
{
    inline const juce::Colour background   { 0xff2a2a2a };   // поле с точками (светлее панели)
    inline const juce::Colour panel        { 0xff1e1e1e };   // правая панель
    inline const juce::Colour panelBorder  { 0xff333333 };
    inline const juce::Colour grid         { 0xff3a3a3a };
    inline const juce::Colour button       { 0xff333333 };
    inline const juce::Colour buttonHover  { 0xff3d3d3d };
    inline const juce::Colour buttonActive { 0xff5a5a5a };
    inline const juce::Colour text         { 0xffd0d0d0 };
    inline const juce::Colour textMuted    { 0xff8a8a8a };
    inline const juce::Colour textHint     { 0xff5c5c5c };
    inline const juce::Colour curve        { 0xffb0b0b0 };
    inline const juce::Colour menu         { 0xff2a2a2a };
    inline const juce::Colour menuHover    { 0xff424242 };

    // цвета ступеней с круга квинт: тёмный вариант (плашки легенды) и светлый (клавиши)
    inline const juce::Colour tonic        { 0xff689461 };   // тоника
    inline const juce::Colour tonicLight   { 0xffa3b58f };
    inline const juce::Colour third        { 0xff8f8869 };   // терция
    inline const juce::Colour fifth        { 0xff697c8f };   // квинта
    inline const juce::Colour seventh      { 0xff7c698f };   // септима
    inline const juce::Colour seventhLight { 0xffa28fb5 };
    inline const juce::Colour ninth        { 0xff698f8f };   // нона
    inline const juce::Colour eleventh     { 0xff8f7c69 };   // ундецима
    inline const juce::Colour thirteenth   { 0xff7d8f69 };   // терцдецима

    // точки кривой
    inline const juce::Colour point         = tonic;
    inline const juce::Colour pointOutline  = tonicLight;
    inline const juce::Colour pointSelected = tonicLight;

    // ручки изгиба отрезка — жёлтые
    inline const juce::Colour handle        { 0xffc9a04a };
    inline const juce::Colour handleBent    { 0xffe0bc63 };
    inline const juce::Colour handleOutline { 0xffeed38f };

    // отметка нажатой ноты на поле
    inline const juce::Colour noteMarker    = tonicLight;
}

//==============================================================================
class PcKeysLookAndFeel : public juce::LookAndFeel_V4
{
public:
    PcKeysLookAndFeel()
        : juce::LookAndFeel_V4 (ColourScheme (juce::Colour (0xff212121),   // фон окон
                                              Theme::button,              // фон элементов
                                              Theme::menu,                // фон меню
                                              Theme::panelBorder,         // обводка
                                              Theme::text,                // текст
                                              Theme::buttonActive,        // заливка
                                              juce::Colours::white,       // выделенный текст
                                              Theme::menuHover,           // выделение
                                              Theme::text))               // текст меню
    {
       #if JUCE_WINDOWS
        setDefaultSansSerifTypefaceName ("Segoe UI");
       #endif

        setColour (juce::TextButton::buttonColourId,   Theme::button);
        setColour (juce::TextButton::buttonOnColourId, Theme::buttonActive);
        setColour (juce::TextButton::textColourOffId,  Theme::text);
        setColour (juce::TextButton::textColourOnId,   Theme::text);

        setColour (juce::Label::textColourId, Theme::text);

        setColour (juce::TextEditor::backgroundColourId,     Theme::button);
        setColour (juce::TextEditor::textColourId,           Theme::text);
        setColour (juce::TextEditor::outlineColourId,        juce::Colours::transparentBlack);
        setColour (juce::TextEditor::focusedOutlineColourId, Theme::buttonActive);
        setColour (juce::TextEditor::highlightColourId,      Theme::buttonActive);
        setColour (juce::CaretComponent::caretColourId,      Theme::text);

        setColour (juce::PopupMenu::backgroundColourId,            Theme::menu);
        setColour (juce::PopupMenu::textColourId,                  Theme::text);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, Theme::menuHover);
        setColour (juce::PopupMenu::highlightedTextColourId,       juce::Colours::white);

        setColour (juce::AlertWindow::backgroundColourId, juce::Colour (0xff212121));
        setColour (juce::AlertWindow::textColourId,       Theme::text);
        setColour (juce::AlertWindow::outlineColourId,    Theme::panelBorder);

        setColour (juce::ScrollBar::thumbColourId, Theme::buttonActive);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour&,
                               bool isHighlighted, bool isDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);

        auto colour = button.getToggleState() ? Theme::buttonActive : Theme::button;

        if (! button.isEnabled())
            colour = colour.withMultipliedAlpha (0.5f);
        else if (isDown)
            colour = Theme::buttonActive;
        else if (isHighlighted && ! button.getToggleState())
            colour = Theme::buttonHover;

        g.setColour (colour);
        g.fillRoundedRectangle (bounds, 6.0f);
    }

    // галка — как в настройках Ear Trainer: квадрат 16x16, при включении серый с белой птичкой
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool isHighlighted, bool) override
    {
        const auto bounds = button.getLocalBounds().toFloat();
        const auto box = juce::Rectangle<float> (0.0f, (bounds.getHeight() - 16.0f) * 0.5f, 16.0f, 16.0f);

        const bool on = button.getToggleState();

        g.setColour (on ? Theme::buttonActive : (isHighlighted ? Theme::buttonHover : Theme::button));
        g.fillRoundedRectangle (box, 3.0f);

        if (on)
        {
            juce::Path tick;
            tick.startNewSubPath (box.getX() + 3.8f, box.getCentreY() + 0.2f);
            tick.lineTo (box.getX() + 6.8f, box.getBottom() - 4.0f);
            tick.lineTo (box.getRight() - 3.5f, box.getY() + 4.0f);

            g.setColour (juce::Colours::white);
            g.strokePath (tick, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));
        }

        g.setColour (Theme::text.withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));
        g.setFont (juce::FontOptions (13.5f));
        g.drawFittedText (button.getButtonText(),
                          bounds.withTrimmedLeft (box.getRight() + 8.0f).toNearestInt(),
                          juce::Justification::centredLeft, 1, 1.0f);
    }

    juce::Font getTextButtonFont (juce::TextButton& button, int buttonHeight) override
    {
        const auto height = juce::jmin (14.0f, (float) buttonHeight * 0.55f);
        return juce::FontOptions (height, button.getToggleState() ? juce::Font::bold : juce::Font::plain);
    }

    juce::AlertWindow* createAlertWindow (const juce::String& title, const juce::String& message,
                                          const juce::String& button1, const juce::String& button2,
                                          const juce::String& button3, juce::MessageBoxIconType iconType,
                                          int numButtons, juce::Component* associatedComponent) override
    {
        auto* window = juce::LookAndFeel_V4::createAlertWindow (title, message, button1, button2, button3,
                                                                iconType, numButtons, associatedComponent);
        window->setLookAndFeel (this);
        return window;
    }

    void drawCornerResizer (juce::Graphics& g, int w, int h, bool isMouseOver, bool isMouseDragging) override
    {
        const auto size = (float) juce::jmin (w, h);
        g.setColour (isMouseOver || isMouseDragging ? Theme::text : Theme::textMuted);

        for (float i = 0.3f; i < 1.0f; i += 0.3f)
            g.drawLine ((float) w - size * i, (float) h, (float) w, (float) h - size * i, 1.2f);
    }
};
