#include <windows.h>
#include "Arena.h"
#include "ConsoleIO.h"

void main(void) {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    Arena arena;
    SIZE_T PageSize = 65536;
    ArenaInit(&arena, PageSize);


    stringW a = StringLiteral(L"Привет, введите своё имя\n");
    PrintStringW(a);

    stringW name = InputStringW(&arena);
    stringW b = C_STR("Вывод: ");
    PrintStringW(b);
    PrintStringW(name);


    Pause();
    ArenaDestroy(&arena);
    ExitProcess(0);
}
