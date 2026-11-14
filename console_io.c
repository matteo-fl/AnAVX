#include <intrin.h> 
#include "console_io.h"

void Pause() {
	HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode; 
	GetConsoleMode(hStdIn, &mode);

	SetConsoleMode(hStdIn, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));

	INPUT_RECORD ir;
	DWORD read;

	while (ReadConsoleInputW(hStdIn, &ir, 1, &read)) {
		if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown) {
			break;
		}
	}

	SetConsoleMode(hStdIn, mode);
}


void PrintInt(int64_t value) {
	char buffer[64] = { 0 };
	string8 str = String8FromInt(buffer, sizeof(buffer), value);
	os_write_stdout(str.data, str.size);
}


//________________________________________________________________________________________
string8 InputString8(Arena* arena) {
	string8 result = { 0 };
	if (!arena) return result;

	// Максимум 4096 символов из os_read_stdin * до 4 байт на символ в UTF-8 + 1 байт под '\0'
	size_t max_bytes = 4096 * 4 + 1;
	size_t start_used = arena->_used;

	char* buf = ArenaPushArray(arena, char, max_bytes);
	if (!buf) return result;

	size_t bytes_read = os_read_stdin(buf, max_bytes - 1);

	// Обрезаем символы переноса строки (\r и \n)
	while (bytes_read > 0 && (buf[bytes_read - 1] == '\r' || buf[bytes_read - 1] == '\n')) {
		bytes_read--;
	}

	buf[bytes_read] = '\0';
	result.data = buf;
	result.size = bytes_read;

	// Возвращаем неиспользованную память обратно в арену (с учетом выравнивания 32 байта)
	arena->_used = start_used + ALIGN_UP(bytes_read + 1, 32);

	return result;
}

void PrintString8(string8 str) {
	if (str.size == 0 || !str.data) return;
	os_write_stdout(str.data, str.size);
}