#pragma once
#include <windows.h>
#include "Arena.h"
#include "string.h"

void Pause();
void PrintStringW(stringW str);
stringW InputStringW(Arena* arena);
void PrintIntInterval(INT64 value); // Оставляем сигнатуру, реализацию напишешь по необходимости
