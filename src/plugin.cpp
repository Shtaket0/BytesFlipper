#include "pluginmain.h"
#include <windows.h>
#include <string>
#include <cctype>

#define IDC_IN  101
#define IDC_BTN 102
#define IDC_OUT 103

static HWND g_hWnd = nullptr;
static WNDPROC g_origEditProc = nullptr;

static std::string FlipBytes(const std::string& raw) {
    std::string h, out;
    for (size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] == '0' && i + 1 < raw.size() && (raw[i + 1] == 'x' || raw[i + 1] == 'X')) {
            ++i;
            continue;
        }
        if (std::isxdigit((unsigned char)raw[i])) {
            h.push_back((char)std::toupper((unsigned char)raw[i]));
        }
    }
    if (h.empty()) return "";
    if (h.size() % 2) h.insert(h.begin(), '0');
    for (size_t i = h.size(); i >= 2; i -= 2) {
        out.push_back(h[i - 2]);
        out.push_back(h[i - 1]);
    }
    return out;
}

static void ProcessInput() {
    HWND hIn = GetDlgItem(g_hWnd, IDC_IN);
    int len = GetWindowTextLengthA(hIn);
    if (len <= 0) {
        SetDlgItemTextA(g_hWnd, IDC_OUT, "");
        return;
    }
    std::string buf(len + 1, '\0');
    GetWindowTextA(hIn, &buf[0], len + 1);
    buf.resize(len);

    std::string res = FlipBytes(buf);
    SetDlgItemTextA(g_hWnd, IDC_OUT, res.c_str());
}

static LRESULT CALLBACK EditProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_KEYDOWN && w == VK_RETURN) {
        ProcessInput();
        return 0;
    }
    if (m == WM_CHAR && w == VK_RETURN) {
        return 0;
    }
    return CallWindowProcA(g_origEditProc, h, m, w, l);
}

static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_CREATE) {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtrA(h, GWLP_HINSTANCE);

        HWND hIn = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 12, 12, 270, 24, h, (HMENU)IDC_IN, hInst, nullptr);
        HWND hBtn = CreateWindowA("BUTTON", "Flip", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 12, 44, 270, 28, h, (HMENU)IDC_BTN, hInst, nullptr);
        HWND hOut = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_READONLY | ES_AUTOHSCROLL, 12, 80, 270, 24, h, (HMENU)IDC_OUT, hInst, nullptr);

        SendMessageA(hIn, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessageA(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessageA(hOut, WM_SETFONT, (WPARAM)hFont, TRUE);

        g_origEditProc = (WNDPROC)SetWindowLongPtrA(hIn, GWLP_WNDPROC, (LONG_PTR)EditProc);
        return 0;
    }
    if (m == WM_COMMAND && LOWORD(w) == IDC_BTN) {
        ProcessInput();
        return 0;
    }
    if (m == WM_CLOSE) {
        ShowWindow(h, SW_HIDE);
        return 0;
    }
    return DefWindowProcA(h, m, w, l);
}

static void cbMenuEntry(CBTYPE cbType, void* callbackInfo) {
    auto* info = (PLUG_CB_MENUENTRY*)callbackInfo;
    if (info->hEntry == 1) {
        if (!g_hWnd) {
            HINSTANCE hInst = nullptr;
            GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&WndProc, &hInst);

            WNDCLASSA wc = { 0, WndProc, 0, 0, hInst, nullptr, LoadCursorA(nullptr, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), nullptr, "BytesFlipper" };
            RegisterClassA(&wc);
            g_hWnd = CreateWindowExA(WS_EX_TOPMOST, wc.lpszClassName, "BytesFlipper", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, 250, 250, 310, 155, hwndDlg, nullptr, hInst, nullptr);
        }
        ShowWindow(g_hWnd, SW_SHOW);
        SetForegroundWindow(g_hWnd);
        SetFocus(GetDlgItem(g_hWnd, IDC_IN));
    }
}

bool pluginInit(PLUG_INITSTRUCT* initStruct) {
    initStruct->pluginVersion = 1;
    initStruct->sdkVersion = PLUG_SDKVERSION;
    strncpy_s(initStruct->pluginName, "BytesFlipper", _TRUNCATE);
    _plugin_registercallback(pluginHandle, CB_MENUENTRY, cbMenuEntry);
    return true;
}

void pluginStop() {
    if (g_hWnd) {
        DestroyWindow(g_hWnd);
        g_hWnd = nullptr;
    }
    HINSTANCE hInst = nullptr;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&WndProc, &hInst);
    UnregisterClassA("BytesFlipper", hInst);
}

void pluginSetup() {
    _plugin_menuaddentry(hMenu, 1, "BytesFlipper");
}