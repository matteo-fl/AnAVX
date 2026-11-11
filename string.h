#pragma once
#include <stddef.h> 
typedef struct Arena Arena;

typedef struct {
    wchar_t* data;
    size_t sym_cnt; 
} stringW;


stringW StringLiteral(const wchar_t* text);
stringW ArenaPushString(Arena* arena, stringW src);
int StringEquals(stringW s1, stringW s2);
stringW StringSlice(stringW str, size_t start, size_t end);
stringW ArenaConcat(Arena* arena, stringW s1, stringW s2);

unsigned char GetRussianCharIndex(const char* utf8_bytes, size_t* out_char_size);

#define C_STR(lit) ((stringW){ (wchar_t*)L##lit, (sizeof(L##lit) / sizeof(wchar_t)) - 1 })
