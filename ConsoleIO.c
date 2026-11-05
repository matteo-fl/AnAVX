#include <windows.h>
#include "consoleIO.h"
#include "Arena.h"
#include "string.h"

void PrintStringW(stringW str) {
    if (str.sym_cnt == 0 || !str.data) return;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written;

    // Выводим строки через нативный юникодный WriteConsoleW
    WriteConsoleW(hOut, str.data, (DWORD)str.sym_cnt, &written, NULL);
}

stringW InputStringW(Arena* arena) {
    stringW result = { 0 };
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);

    size_t maxChars = 65536;

    // Используем очищенный макрос выделения памяти Арены под wchar_t
    result.data = ArenaPushArray(arena, wchar_t, maxChars + 1);
    if (!result.data) return result;

    DWORD charsRead = 0;

    if (ReadConsoleW(hIn, result.data, (DWORD)maxChars, &charsRead, NULL)) {
        // Очищаем хвост от символов переноса строки (Enter)
        while (charsRead > 0 && (result.data[charsRead - 1] == L'\r' || result.data[charsRead - 1] == L'\n')) {
            charsRead--;
        }

        result.sym_cnt = charsRead;
        result.data[charsRead] = L'\0'; // Ставим гарантированный нуль-терминатор
    }

    return result;
}

void Pause() {
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdIn, &mode);

    // Отключаем построчный ввод и эхо символов, чтобы поймать одиночное нажатие клавиши
    SetConsoleMode(hStdIn, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));

    INPUT_RECORD ir;
    DWORD read;

    while (ReadConsoleInputW(hStdIn, &ir, 1, &read)) {
        if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown) {
            break;
        }
    }

    // Восстанавливаем исходный режим консоли обратно
    SetConsoleMode(hStdIn, mode);
}
