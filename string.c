#include <windows.h>
#include "string.h"
#include "Arena.h" // Подключаем Арену здесь, так как внутри нужны макросы аллокации

stringW StringLiteral(const wchar_t* text) {
    stringW str;
    str.data = (wchar_t*)text;
    str.sym_cnt = text ? (size_t)lstrlenW(text) : 0; // Чистый Win32 вызов без CRT
    return str;
}

int StringEquals(stringW s1, stringW s2) {
    if (s1.sym_cnt != s2.sym_cnt) return 0; // FALSE
    if (s1.data == s2.data)       return 1; // TRUE

    for (size_t i = 0; i < s1.sym_cnt; i++) {
        if (s1.data[i] != s2.data[i]) return 0;
    }
    return 1; // TRUE
}

stringW StringSlice(stringW str, size_t start, size_t end) {
    if (start > str.sym_cnt) start = str.sym_cnt;
    if (end > str.sym_cnt)   end = str.sym_cnt;
    if (start > end)         start = end;

    stringW result;
    result.data = str.data + start;
    result.sym_cnt = end - start;
    return result;
}

stringW ArenaPushString(Arena* arena, stringW src) {
    stringW result;
    result.sym_cnt = src.sym_cnt;

    // Выделяем память под массив широких символов + 1 под нуль-терминатор
    result.data = ArenaPushArray(arena, wchar_t, src.sym_cnt + 1);

    if (result.data) {
        for (size_t i = 0; i < src.sym_cnt; i++) {
            result.data[i] = src.data[i];
        }
        result.data[src.sym_cnt] = L'\0';
    }
    return result;
}

stringW ArenaConcat(Arena* arena, stringW s1, stringW s2) {
    stringW result;
    result.sym_cnt = s1.sym_cnt + s2.sym_cnt;

    result.data = ArenaPushArray(arena, wchar_t, result.sym_cnt + 1);

    if (result.data) {
        size_t offset = 0;
        for (size_t i = 0; i < s1.sym_cnt; i++) result.data[offset++] = s1.data[i];
        for (size_t i = 0; i < s2.sym_cnt; i++) result.data[offset++] = s2.data[i];
        result.data[offset] = L'\0';
    }
    return result;
}
