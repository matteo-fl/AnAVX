#pragma once
#include "arena.h"
#include "string.h"

void Pause();
void PrintStringW(stringW str);
stringW InputStringW(Arena* arena);
void PrintInt(signed long long value);
void AVX_PrintInt(signed long long value);
