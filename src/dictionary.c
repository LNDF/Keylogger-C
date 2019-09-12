#include "dictionary.h"

keyDictionaryEntry keyDictionary[234];
u8 keyDictionaryEntries = 0;

void constructKeyDictionary() {
    memset(keyDictionary, 0, sizeof(keyDictionaryEntry) * 234);
    for (u16 i = 0x21; i <= 255; i++) {
        if (i == 0x7F) continue;
        short kd = VkKeyScan(i);
        char keyCOde = kd & 0xFF;
        if (keyCOde < 0) continue;
        u8 shift = 0, control = 0, alt = 0;
        if (kd & 0x100) shift = 1;
        if (kd & 0x200) control = 1;
        if (kd & 0x400) alt = 1;
        keyDictionaryEntry current = {0};
        current.chr = i;
        current.vkCode = keyCOde;
        current.shift = shift;
        current.control = control;
        current.alt = alt;
        keyDictionary[keyDictionaryEntries] = current;
        keyDictionaryEntries++;
    }
}
