#pragma once

#include <juce_core/juce_core.h>

//==============================================================================
//  Все надписи интерфейса на двух языках.
//==============================================================================
struct Texts
{
    juce::String layout, piano, grid, scale, tonic;
    juce::String octave, transpose, velocity, sustain;
    juce::String capture, captureTip, hideAll, hideTip, resetTip;
    juce::String sounding, hint;
    juce::StringArray modes;

    static juce::String u (const char* s) { return juce::String::fromUTF8 (s); }

    static Texts get (int language)
    {
        Texts t;

        if (language == 0)
        {
            t.layout     = u ("Раскладка");
            t.piano      = u ("Пианино");
            t.grid       = u ("Сетка");
            t.scale      = u ("Только ноты лада");
            t.tonic      = u ("Тоника");
            t.octave     = u ("Октава");
            t.transpose  = u ("Полутоны");
            t.velocity   = u ("Velocity");
            t.sustain    = u ("Сустейн (пробел)");
            t.capture    = u ("Перехватывать звук вне окна");
            t.captureTip = u ("Ноты играются с клавиатуры ПК, даже когда активно окно DAW или другой программы, "
                              "а не окно плагина. Горячие клавиши DAW на этих же клавишах при этом тоже срабатывают.");
            t.hideAll    = u ("Скрыть все настройки");
            t.hideTip    = u ("Скрывает всё, кроме клавиш. По нажатию Esc все настройки появляются опять.");
            t.resetTip   = u ("Нажмите на название, чтобы вернуть значение по умолчанию.");
            t.sounding   = u ("Звучит:");
            t.hint       = u ("← → октава    ↑ ↓ полутон    пробел — сустейн    Esc — вернуть скрытые настройки");

            t.modes = { u ("мажор"), u ("минор"), u ("дорийский"), u ("фригийский"), u ("лидийский"),
                        u ("миксолидийский"), u ("локрийский"), u ("гармонический минор"), u ("мелодический минор") };
        }
        else
        {
            t.layout     = u ("Layout");
            t.piano      = u ("Piano");
            t.grid       = u ("Grid");
            t.scale      = u ("Scale notes only");
            t.tonic      = u ("Tonic");
            t.octave     = u ("Octave");
            t.transpose  = u ("Semitones");
            t.velocity   = u ("Velocity");
            t.sustain    = u ("Sustain (space)");
            t.capture    = u ("Capture sound outside window");
            t.captureTip = u ("Notes are played from the PC keyboard even when the DAW or another program is active "
                              "instead of the plugin window. DAW hotkeys on the same keys still fire as well.");
            t.hideAll    = u ("Hide all settings");
            t.hideTip    = u ("Hides everything except the keys. Press Esc and all settings appear again.");
            t.resetTip   = u ("Click the name to return to the default value.");
            t.sounding   = u ("Sounding:");
            t.hint       = u ("← → octave    ↑ ↓ semitone    space — sustain    Esc — bring hidden settings back");

            t.modes = { "major", "minor", "dorian", "phrygian", "lydian",
                        "mixolydian", "locrian", "harmonic minor", "melodic minor" };
        }

        return t;
    }
};
