#include <windows.h>

void main(void) {
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdout != INVALID_HANDLE_VALUE) {
        const char* msg = "Hello from pure C without CRT!\n";
        WriteFile(hStdout, msg, 31, NULL, NULL);
    }

    ExitProcess(0);
}
