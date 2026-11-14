#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct OsMappedFile {
	const char* data;
	size_t size;
	uintptr_t _handle1;
	uintptr_t _handle2;
} OsMappedFile;

// Виртуальная память
void* os_reserve(size_t size);
int   os_commit(void* ptr, size_t size);
int   os_release(void* ptr, size_t size);

// Консольный ввод-вывод
void   os_write_stdout(const char* data, size_t size);
size_t os_read_stdin(char* buffer, size_t capacity);
void   os_pause(void);

// Отображение файлов в память (Memory-Mapped Files)
int  os_map_file_readonly(OsMappedFile* out_file, const char* path);
void os_unmap_file(OsMappedFile* file);
void os_prefetch_memory(const void* ptr, size_t size);
