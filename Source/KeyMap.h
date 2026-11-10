#pragma once

#include <array>

//==============================================================================
//  Клавиши ПК, которыми играют, и три раскладки нот.
//  Клавиши заданы по месту на клавиатуре, язык ввода значения не имеет.
//==============================================================================
namespace KeyMap
{
    constexpr int numRows = 4;
    constexpr int numKeys = 45;
    constexpr int rowLength[numRows] = { 12, 12, 11, 10 };
    constexpr float rowShift[numRows] = { 0.0f, 0.5f, 0.75f, 1.25f };   // сдвиг ряда в ширинах клавиши

    enum Layout { piano = 0, grid = 1, scale = 2 };

    constexpr int numModes = 9;
    constexpr int modes[numModes][7] =
    {
        { 0, 2, 4, 5, 7, 9, 11 },   // мажор
        { 0, 2, 3, 5, 7, 8, 10 },   // минор
        { 0, 2, 3, 5, 7, 9, 10 },   // дорийский
        { 0, 1, 3, 5, 7, 8, 10 },   // фригийский
        { 0, 2, 4, 6, 7, 9, 11 },   // лидийский
        { 0, 2, 4, 5, 7, 9, 10 },   // миксолидийский
        { 0, 1, 3, 5, 6, 8, 10 },   // локрийский
        { 0, 2, 3, 5, 7, 8, 11 },   // гармонический минор
        { 0, 2, 3, 5, 7, 9, 11 }    // мелодический минор
    };

    struct Key
    {
        int row = 0, col = 0;
        unsigned scanCode = 0;     // номер клавиши на клавиатуре
        unsigned defaultVk = 0;    // код клавиши для обычной раскладки
        char cap = 0;              // что написано на клавише
        int pianoOffset = -1;      // нота в раскладке «Пианино» (-1 — клавиша свободна)
    };

    inline std::array<Key, numKeys> buildKeys()
    {
        const char* caps[numRows] = { "1234567890-=", "QWERTYUIOP[]", "ASDFGHJKL;'", "ZXCVBNM,./" };
        const unsigned firstScan[numRows] = { 0x02, 0x10, 0x1E, 0x2C };

        // «Пианино»: белые клавиши — нижний ряд и ряд Q, чёрные — ряды над ними
        const int pianoRows[numRows][12] =
        {
            { -1, 13, 15, -1, 18, 20, 22, -1, 25, 27, -1, -1 },
            { 12, 14, 16, 17, 19, 21, 23, 24, 26, 28, -1, -1 },
            { -1,  1,  3, -1,  6,  8, 10, -1, -1, -1, -1, -1 },
            {  0,  2,  4,  5,  7,  9, 11, -1, -1, -1, -1, -1 }
        };

        std::array<Key, numKeys> result {};
        int index = 0;

        for (int row = 0; row < numRows; ++row)
        {
            for (int col = 0; col < rowLength[row]; ++col)
            {
                Key k;
                k.row = row;
                k.col = col;
                k.scanCode = firstScan[row] + (unsigned) col;
                k.cap = caps[row][col];
                k.pianoOffset = pianoRows[row][col];

                switch (k.cap)
                {
                    case '-':  k.defaultVk = 0xBD; break;
                    case '=':  k.defaultVk = 0xBB; break;
                    case '[':  k.defaultVk = 0xDB; break;
                    case ']':  k.defaultVk = 0xDD; break;
                    case ';':  k.defaultVk = 0xBA; break;
                    case '\'': k.defaultVk = 0xDE; break;
                    case ',':  k.defaultVk = 0xBC; break;
                    case '.':  k.defaultVk = 0xBE; break;
                    case '/':  k.defaultVk = 0xBF; break;
                    default:   k.defaultVk = (unsigned) k.cap; break;
                }

                result[(size_t) index++] = k;
            }
        }

        return result;
    }

    inline const std::array<Key, numKeys>& keys()
    {
        static const std::array<Key, numKeys> all = buildKeys();
        return all;
    }

    // нота клавиши при текущих настройках; -1 — клавиша свободна или нота вне 0…127
    inline int noteFor (const Key& key, int layout, int tonic, int mode, int octave, int transpose)
    {
        const int fromBottom = numRows - 1 - key.row;
        int note = 0;

        if (layout == piano)
        {
            if (key.pianoOffset < 0)
                return -1;

            note = 48 + key.pianoOffset;                       // нижняя нота — C3
        }
        else if (layout == grid)
        {
            note = 36 + fromBottom * 5 + key.col * 2;          // нижняя нота — C2
        }
        else
        {
            const int m = (mode >= 0 && mode < numModes) ? mode : 0;
            note = 36 + tonic + 12 * fromBottom + 12 * (key.col / 7) + modes[m][key.col % 7];
        }

        note += octave * 12 + transpose;
        return (note < 0 || note > 127) ? -1 : note;
    }

    inline bool isRoot (const Key& key, int layout)   { return layout == scale && key.col % 7 == 0; }
}
