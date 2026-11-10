#include "WinKeys.h"

#ifdef _WIN32

#ifndef NOMINMAX
 #define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>

namespace WinKeys
{
    void beginFineTimer() { timeBeginPeriod (1); }
    void endFineTimer()   { timeEndPeriod (1); }

    bool isWindowActive (void* editorWindow)
    {
        const auto window = (HWND) editorWindow;

        if (window == nullptr || ! IsWindow (window))
            return false;

        const auto foreground = GetForegroundWindow();
        return foreground != nullptr && foreground == GetAncestor (window, GA_ROOT);
    }

    bool isModifierDown()
    {
        return (GetAsyncKeyState (VK_CONTROL) & 0x8000) != 0
            || (GetAsyncKeyState (VK_MENU) & 0x8000) != 0
            || (GetAsyncKeyState (VK_LWIN) & 0x8000) != 0
            || (GetAsyncKeyState (VK_RWIN) & 0x8000) != 0;
    }

    void updateKeyCodes (void*& lastLayout, const unsigned* scanCodes, const unsigned* defaults,
                         unsigned* codes, int count)
    {
        const auto foreground = GetForegroundWindow();
        const auto thread = foreground != nullptr ? GetWindowThreadProcessId (foreground, nullptr) : 0;
        const auto layout = GetKeyboardLayout (thread);

        if (lastLayout == (void*) layout && lastLayout != nullptr)
            return;

        lastLayout = (void*) layout;

        for (int i = 0; i < count; ++i)
        {
            const auto code = MapVirtualKeyExW (scanCodes[i], MAPVK_VSC_TO_VK, layout);
            codes[i] = code != 0 ? code : defaults[i];
        }
    }

    bool isKeyDown (unsigned code)
    {
        return code != 0 && (GetAsyncKeyState ((int) code) & 0x8000) != 0;
    }

    bool isSpecialDown (Special key)
    {
        static const int codes[numSpecial] = { VK_SPACE, VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN, VK_ESCAPE };
        return (GetAsyncKeyState (codes[key]) & 0x8000) != 0;
    }
}

#else

// не Windows: клавиши ПК не читаются (плагин собирается только под Windows)
namespace WinKeys
{
    void beginFineTimer() {}
    void endFineTimer() {}
    bool isWindowActive (void*) { return false; }
    bool isModifierDown() { return false; }

    void updateKeyCodes (void*&, const unsigned*, const unsigned* defaults, unsigned* codes, int count)
    {
        for (int i = 0; i < count; ++i)
            codes[i] = defaults[i];
    }

    bool isKeyDown (unsigned) { return false; }
    bool isSpecialDown (Special) { return false; }
}

#endif
