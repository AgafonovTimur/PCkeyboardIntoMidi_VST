#pragma once

//==============================================================================
//  Обращения к Windows: какие клавиши ПК сейчас нажаты и какое окно активно.
//  Вынесено отдельно, чтобы заголовки Windows не смешивались с остальным кодом.
//==============================================================================
namespace WinKeys
{
    enum Special { space = 0, left, right, up, down, escape, numSpecial };

    void beginFineTimer();
    void endFineTimer();

    // активно ли окно, в котором открыт плагин
    bool isWindowActive (void* editorWindow);

    // зажаты Ctrl, Alt или Win — это сочетания клавиш, ноты по ним не играем
    bool isModifierDown();

    // пересчитывает коды клавиш, если сменилась раскладка клавиатуры
    void updateKeyCodes (void*& lastLayout, const unsigned* scanCodes, const unsigned* defaults,
                         unsigned* codes, int count);

    bool isKeyDown (unsigned code);
    bool isSpecialDown (Special key);
}
