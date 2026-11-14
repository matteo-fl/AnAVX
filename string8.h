#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct string8 {
	const char* data;
	size_t size;
} string8;

#define STR8(s) ((string8){ (s), sizeof(s) - 1 })

string8       String8Literal(const char* text);
int           String8Equals(string8 s1, string8 s2);
string8       String8Slice(string8 str, size_t start, size_t end);
int           String8Concat(void* buffer, size_t buffer_size, string8 s1, string8 s2, string8* result);
unsigned char GetRussianCharIndex(const char* utf8_bytes, size_t* out_char_size);
string8       String8FromInt(char* buffer, size_t capacity, int64_t value);
int           String8ToInt(string8 str);
