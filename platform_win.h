#include "platform.h"

// ============================================================================
// Константы Windows API (из winnt.h, memoryapi.h, fileapi.h, consoleapi.h)
// ============================================================================
#define MEM_COMMIT            0x00001000
#define MEM_RESERVE           0x00002000
#define MEM_RELEASE           0x00008000
#define PAGE_NOACCESS         0x01
#define PAGE_READONLY         0x02
#define PAGE_READWRITE        0x04

#define GENERIC_READ          0x80000000
#define FILE_SHARE_READ       0x00000001
#define OPEN_EXISTING         3
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define FILE_MAP_READ         0x0004

#define STD_INPUT_HANDLE      ((uint32_t)-10)
#define STD_OUTPUT_HANDLE     ((uint32_t)-11)
#define ENABLE_LINE_INPUT     0x0002
#define ENABLE_ECHO_INPUT     0x0004
#define KEY_EVENT             0x0001
#define CP_UTF8               65001

#define INVALID_HANDLE_VAL    ((void*)(intptr_t)-1)

// ============================================================================
// Минимальные структуры WinAPI для ввода с консоли
// ============================================================================
typedef struct WinInputRecord {
	uint16_t EventType;
	uint16_t _padding;
	union {
		struct {
			int32_t  bKeyDown;
			uint16_t wRepeatCount;
			uint16_t wVirtualKeyCode;
			uint16_t wVirtualScanCode;
			uint16_t UnicodeChar;
			uint32_t dwControlKeyState;
		} KeyEvent;
		uint8_t _raw[16];
	} Event;
} WinInputRecord;

// ============================================================================
// Прямые импорты функций из kernel32.dll (без подключения windows.h)
// ============================================================================
__declspec(dllimport) void* __stdcall VirtualAlloc(void* lpAddress, size_t dwSize, uint32_t flAllocationType, uint32_t flProtect);
__declspec(dllimport) int      __stdcall VirtualFree(void* lpAddress, size_t dwSize, uint32_t dwFreeType);

__declspec(dllimport) void* __stdcall GetStdHandle(uint32_t nStdHandle);
__declspec(dllimport) int      __stdcall SetConsoleCP(uint32_t wCodePageID);
__declspec(dllimport) int      __stdcall SetConsoleOutputCP(uint32_t wCodePageID);
__declspec(dllimport) int      __stdcall GetConsoleMode(void* hConsoleHandle, uint32_t* lpMode);
__declspec(dllimport) int      __stdcall SetConsoleMode(void* hConsoleHandle, uint32_t dwMode);
__declspec(dllimport) int      __stdcall ReadConsoleW(void* hConsoleInput, void* lpBuffer, uint32_t nNumberOfCharsToRead, uint32_t* lpNumberOfCharsRead, void* pInputControl);
__declspec(dllimport) int      __stdcall ReadConsoleInputW(void* hConsoleInput, WinInputRecord* lpBuffer, uint32_t nLength, uint32_t* lpNumberOfEventsRead);
__declspec(dllimport) int      __stdcall WriteFile(void* hFile, const void* lpBuffer, uint32_t nNumberOfBytesToWrite, uint32_t* lpNumberOfBytesWritten, void* lpOverlapped);

__declspec(dllimport) int      __stdcall MultiByteToWideChar(uint32_t CodePage, uint32_t dwFlags, const char* lpMultiByteStr, int cbMultiByte, uint16_t* lpWideCharStr, int cchWideChar);
__declspec(dllimport) int      __stdcall WideCharToMultiByte(uint32_t CodePage, uint32_t dwFlags, const uint16_t* lpWideCharStr, int cchWideChar, char* lpMultiByteStr, int cbMultiByte, const char* lpDefaultChar, int* lpUsedDefaultChar);

__declspec(dllimport) void* __stdcall CreateFileW(const uint16_t* lpFileName, uint32_t dwDesiredAccess, uint32_t dwShareMode, void* lpSecurityAttributes, uint32_t dwCreationDisposition, uint32_t dwFlagsAndAttributes, void* hTemplateFile);
__declspec(dllimport) int      __stdcall GetFileSizeEx(void* hFile, int64_t* lpFileSize);
__declspec(dllimport) void* __stdcall CreateFileMappingW(void* hFile, void* lpFileMappingAttributes, uint32_t flProtect, uint32_t dwMaximumSizeHigh, uint32_t dwMaximumSizeLow, const uint16_t* lpName);
__declspec(dllimport) void* __stdcall MapViewOfFile(void* hFileMappingObject, uint32_t dwDesiredAccess, uint32_t dwFileOffsetHigh, uint32_t dwFileOffsetLow, size_t dwNumberOfBytesToMap);
__declspec(dllimport) int      __stdcall UnmapViewOfFile(const void* lpBaseAddress);
__declspec(dllimport) int      __stdcall CloseHandle(void* hObject);

__declspec(dllimport) int      __stdcall QueryPerformanceFrequency(int64_t* lpFrequency);
__declspec(dllimport) int      __stdcall QueryPerformanceCounter(int64_t* lpPerformanceCount);
__declspec(dllimport) void     __stdcall ExitProcess(uint32_t uExitCode);

// ============================================================================
// Инициализация, таймер и завершение процесса
// ============================================================================
static int64_t g_perf_freq = 0;

void os_init(void) {
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
	QueryPerformanceFrequency(&g_perf_freq);
}

void os_exit(int code) {
	ExitProcess((uint32_t)code);
}

uint64_t os_time_ns(void) {
	if (g_perf_freq == 0) {
		QueryPerformanceFrequency(&g_perf_freq);
	}
	int64_t counter = 0;
	QueryPerformanceCounter(&counter);

	// Разделяем целую и дробную части секунд, чтобы избежать переполнения 64-битного числа
	uint64_t sec = (uint64_t)(counter / g_perf_freq);
	uint64_t rem = (uint64_t)(counter % g_perf_freq);
	return sec * 1000000000ULL + (rem * 1000000000ULL) / (uint64_t)g_perf_freq;
}

// ============================================================================
// Виртуальная память
// ============================================================================
void* os_reserve(size_t size) {
	return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
}

int os_commit(void* ptr, size_t size) {
	if (VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) == NULL) {
		return 1; // Ошибка
	}
	return 0; // Успех
}

int os_release(void* ptr, size_t size) {
	(void)size;
	if (!ptr) {
		return 1;
	}
	if (VirtualFree(ptr, 0, MEM_RELEASE) == 0) {
		return 1;
	}
	return 0;
}

// ============================================================================
// Консольный ввод-вывод
// ============================================================================
void os_write_stdout(const char* data, size_t size) {
	if (!data || size == 0) {
		return;
	}

	void* hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (!hOut || hOut == INVALID_HANDLE_VAL) {
		return;
	}

	size_t total_written = 0;
	while (total_written < size) {
		size_t remaining = size - total_written;
		uint32_t to_write = remaining > 0xFFFFFFFF ? 0xFFFFFFFF : (uint32_t)remaining;
		uint32_t written = 0;

		if (!WriteFile(hOut, data + total_written, to_write, &written, NULL) || written == 0) {
			break;
		}
		total_written += (size_t)written;
	}
}

size_t os_read_stdin(char* buffer, size_t capacity) {
	if (!buffer || capacity == 0) {
		return 0;
	}

	void* hIn = GetStdHandle(STD_INPUT_HANDLE);
	if (!hIn || hIn == INVALID_HANDLE_VAL) {
		return 0;
	}

	uint16_t wide_buf[4096];
	uint32_t max_chars = (uint32_t)(capacity < 4096 ? capacity : 4096);
	uint32_t chars_read = 0;

	if (!ReadConsoleW(hIn, wide_buf, max_chars, &chars_read, NULL) || chars_read == 0) {
		return 0;
	}

	int bytes_written = WideCharToMultiByte(
		CP_UTF8, 0,
		wide_buf, (int)chars_read,
		buffer, (int)capacity,
		NULL, NULL
	);

	return bytes_written > 0 ? (size_t)bytes_written : 0;
}

void os_pause(void) {
	void* hStdIn = GetStdHandle(STD_INPUT_HANDLE);
	if (!hStdIn || hStdIn == INVALID_HANDLE_VAL) {
		return;
	}

	uint32_t mode = 0;
	if (!GetConsoleMode(hStdIn, &mode)) {
		return;
	}

	SetConsoleMode(hStdIn, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));

	WinInputRecord ir;
	uint32_t read = 0;

	while (ReadConsoleInputW(hStdIn, &ir, 1, &read)) {
		if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown) {
			break;
		}
	}

	SetConsoleMode(hStdIn, mode);
}

// ============================================================================
// Отображение файлов в память (Memory-Mapped Files)
// ============================================================================
int os_map_file_readonly(OsMappedFile* out_file, const char* path) {
	if (!out_file || !path) {
		return 1;
	}
	*out_file = (OsMappedFile){ 0 };

	uint16_t wide_path[4096];
	if (MultiByteToWideChar(CP_UTF8, 0, path, -1, wide_path, 4096) == 0) {
		return 1;
	}

	void* file = CreateFileW(wide_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VAL) {
		return 1;
	}

	int64_t size = 0;
	if (!GetFileSizeEx(file, &size) || size <= 0) {
		CloseHandle(file);
		return 1;
	}

	void* file_map = CreateFileMappingW(file, NULL, PAGE_READONLY, 0, 0, NULL);
	if (!file_map) {
		CloseHandle(file);
		return 1;
	}

	const char* data = (const char*)MapViewOfFile(file_map, FILE_MAP_READ, 0, 0, 0);
	if (!data) {
		CloseHandle(file_map);
		CloseHandle(file);
		return 1;
	}

	out_file->data = data;
	out_file->size = (size_t)size;
	out_file->_handle1 = (uintptr_t)file;
	out_file->_handle2 = (uintptr_t)file_map;
	return 0;
}

void os_unmap_file(OsMappedFile* file) {
	if (!file || !file->data) {
		return;
	}

	UnmapViewOfFile(file->data);
	CloseHandle((void*)file->_handle2);
	CloseHandle((void*)file->_handle1);
	*file = (OsMappedFile){ 0 };
}

// ============================================================================
// Точка входа без CRT (для MSVC /NODEFAULTLIB)
// ============================================================================
extern int main(void);

void mainCRTStartup(void) {
	int code = main();
	ExitProcess((uint32_t)code);
}
