#include "dictionary.h"

keyDictionaryEntry keyDictionary[234]; //this is a key dictionary (keycode: equivalent character)
u8 keyDictionaryEntries = 0; //the amount of dictionary entries

/*
    This file builds a key dictionary for the current keyboard layout
*/
void constructKeyDictionary() {
    log("Constructing the keyboard dictionary...");
    memset(keyDictionary, 0, sizeof(keyDictionaryEntry) * 234); //empty the keyboard dictionary, 234 is the max possible entry count
    for (u16 i = 0x21; i <= 255; i++) { //loop through all the possible entry points
        if (i == 0x7F) continue; //exclude some key
        //get the key code for a character
        short kd = VkKeyScan(i);
        char keyCode = kd & 0xFF;
        if (keyCode < 0) continue; //if the character has no key code, continue with the next character
        //check if shift, control or alt needs to be pressed to print this character
        u8 shift = 0, control = 0, alt = 0;
        if (kd & 0x100) shift = 1;
        if (kd & 0x200) control = 1;
        if (kd & 0x400) alt = 1;
        //create a new keyDictionaryEntry structure and fill it with data
        keyDictionaryEntry current = {0};
        current.chr = i;
        current.vkCode = keyCode;
        current.shift = shift;
        current.control = control;
        current.alt = alt;
        //append the keyDictionaryEntry to the key dictionary
        keyDictionary[keyDictionaryEntries] = current;
        keyDictionaryEntries++;
    }
    log("%u entries added to the dictionary.", keyDictionaryEntries);
}

/*
    Using the key dictionary built by the abobe function, get a equivalent character from the key combination
    vkCode: the key
    isControlPressed: 1 if control is pressed, 0 otherwise
    isAltPressed: 1 if alt is pressed, 0 otherwise
    isShiftPressed: 1 if shift is pressed, 0 otherwise
*/
char getCharFromCombination(u32 vkCOde, u8 isControlPressed, u8 isAltPressed, u8 isShiftPressed) {
    for (u8 i = 0; i < keyDictionaryEntries; i++) { //loop through all the dictionary to find a mach
        keyDictionaryEntry kde = keyDictionary[i];
        if (kde.vkCode == vkCOde && kde.alt == isAltPressed && kde.control == isControlPressed && kde.shift == isShiftPressed) return kde.chr;
    }
    return 0; //if a match was not found, return 0
}
