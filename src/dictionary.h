#ifndef INCLUDE_DICTIONARY_H
#define INCLUDE_DICTIONARY_H
#pragma once
#include "keylogger.h"

typedef struct { //a structure to define a key dictionary entry
    u8 chr; //the character
    u32 vkCode; //the key code
    u8 alt; //alt pressed flag
    u8 control; //control pressed flag
    u8 shift; //shift pressed flag
} keyDictionaryEntry;

void constructKeyDictionary();
char getCharFromCombination(u32 vkCOde, u8 isControlPressed, u8 isAltPressed, u8 isShiftPressed);

#endif // DICTIONARY_H
