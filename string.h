#pragma once
#include <stddef.h> // Для чистого size_t без CRT и без windows.h

typedef struct {
    wchar_t* data;
    size_t sym_cnt; // Длина строки (количество СИМВОЛОВ)
} stringW;

// Предварительное объявление структуры Arena, чтобы не делать циклический импорт
struct Arena;

stringW StringLiteral(const wchar_t* text);
stringW ArenaPushString(struct Arena* arena, stringW src);
int StringEquals(stringW s1, stringW s2);
stringW StringSlice(stringW str, size_t start, size_t end);
stringW ArenaConcat(struct Arena* arena, stringW s1, stringW s2);

// Макрос автоматически добавляет L ко всем литералам. Передавай строку без L, например: C_STR("Вывод: ")
#define C_STR(lit) ((stringW){ (wchar_t*)L##lit, (sizeof(L##lit) / sizeof(wchar_t)) - 1 })
