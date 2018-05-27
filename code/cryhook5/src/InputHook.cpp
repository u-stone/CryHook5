
/*
*  This source file is part of the Far Cry 5 ScriptHook by Force67
*  More information regarding licensing can be found in LICENSE.md
*/

#include <Nomad/nomad_base_function.h>
#include <Hooking.h>

#include <imgui.h>

#include <CryHook5.h>

bool input::g_showmenu = false;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static WNDPROC GameWndProc_Orig;

class CMouseStuff
{
public:

    char pad[0x3D9];
    bool bDisableTracking;
};

static CMouseStuff **g_mousestuff;

static LRESULT GameWndProc_Hook(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_KEYUP && wParam == VK_F8)
    {
        input::g_showmenu = !input::g_showmenu;
        if (*g_mousestuff) (*g_mousestuff)->bDisableTracking = input::g_showmenu;
    }

    if (input::g_showmenu)
    {
        ImGuiIO& io = ImGui::GetIO();
        switch (msg)
        {
        case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
        {
            int button = 0;
            if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) button = 0;
            if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) button = 1;
            if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) button = 2;
            io.MouseDown[button] = true;
            return true;
        }
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        {
            int button = 0;
            if (msg == WM_LBUTTONUP) button = 0;
            if (msg == WM_RBUTTONUP) button = 1;
            if (msg == WM_MBUTTONUP) button = 2;
            io.MouseDown[button] = false;
            return true;
        }
        case WM_MOUSEWHEEL:
            io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
            return true;
        case WM_MOUSEHWHEEL:
            io.MouseWheelH += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
            return true;
        case WM_MOUSEMOVE:
            io.MousePos.x = (signed short)(lParam);
            io.MousePos.y = (signed short)(lParam >> 16);
            return true;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if (wParam < 256)
                io.KeysDown[wParam] = 1;
            return true;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (wParam < 256)
                io.KeysDown[wParam] = 0;
            return true;
        case WM_CHAR:
            // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
            if (wParam > 0 && wParam < 0x10000)
                io.AddInputCharacter((unsigned short)wParam);
            return true;
        }

        return true;
    }

    return CallWindowProc(GameWndProc_Orig, hwnd, msg, wParam, lParam);
}

static nomad::base_function init([]()
{

    auto loc = nio::pattern("31 D2 48 8D 4C 24 ? 44 8D 42 50").first(16 + 3);

    // as it gets put into rax
    GameWndProc_Orig = (WNDPROC)(loc + *(int32_t*)loc + 4);
    *(int32_t*)loc = (intptr_t)(nio::AllocateFunctionStub(GameWndProc_Hook)) - (intptr_t)loc - 4;

    // cmouse stuff
    loc = nio::pattern("C6 80 ? ? ? ? ? 48 8B 8B ? ? ? ? 48 85 C9 74 17").first(-4);
    g_mousestuff = (CMouseStuff**)(loc + *(int32_t*)loc + 4);

    //nio::return_function(0x1870C7D50);
  //  nio::put_ljump(0x18B8C3790, BP);

    //18228B8A4  mouse 
    //cursor texture update pos


    // no dinput creation
    //nio::return_function(0x18625D6F0);

 //   nio::nop(0x182FC86D6, 4);

 //   nio::nop(0x182FC8766, 6);
 //   nio::put_call(0x182FC8766, SetCursor_Hook);
});
