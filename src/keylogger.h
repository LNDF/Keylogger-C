#ifndef INCLUDE_KEYLOGGER_H
#define INCLUDE_KEYLOGGER_H
#pragma once
#include <stdlib.h>
#include <inttypes.h>
#include <windows.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define log(...) {printf("[%s]: ", __FUNCTION__); printf(__VA_ARGS__); printf("\n");}

typedef void (*keyloggerCallback)(char* keyStr);
void startKeylogger(HINSTANCE hInstance, keyloggerCallback cb);


#endif // INCLUDE_KEYLOGGER_H
