#pragma once
#include "platform.h"
#include "arena.h"
#include "string8.h"

void    Pause(void);
void    PrintInt(int64_t value);
void    PrintString8(string8 str);
string8 InputString8(Arena* arena);
