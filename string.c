#include <windows.h>
#include "string.h"
#include "arena.h" 
stringW StringLiteral(const wchar_t* text) {
	stringW str;
	str.data = (wchar_t*)text;
	str.sym_cnt = text ? (size_t)lstrlenW(text) : 0;     return str;
}

int StringEquals(stringW s1, stringW s2) {
	if (s1.sym_cnt != s2.sym_cnt) return 0;     if (s1.data == s2.data)       return 1;
	for (size_t i = 0; i < s1.sym_cnt; i++) {
		if (s1.data[i] != s2.data[i]) return 0;
	}
	return 1;
}

stringW StringSlice(stringW str, size_t start, size_t end) {
	if (start > str.sym_cnt) start = str.sym_cnt;
	if (end > str.sym_cnt)   end = str.sym_cnt;
	if (start > end)         start = end;

	stringW result;
	result.data = str.data + start;
	result.sym_cnt = end - start;
	return result;
}

stringW ArenaPushString(Arena* arena, stringW src) {
	stringW result;
	result.sym_cnt = src.sym_cnt;

	result.data = ArenaPushArray(arena, wchar_t, src.sym_cnt + 1);

	if (result.data) {
		for (size_t i = 0; i < src.sym_cnt; i++) {
			result.data[i] = src.data[i];
		}
		result.data[src.sym_cnt] = L'\0';
	}
	return result;
}

stringW ArenaConcat(Arena* arena, stringW s1, stringW s2) {
	stringW result;
	result.sym_cnt = s1.sym_cnt + s2.sym_cnt;

	result.data = ArenaPushArray(arena, wchar_t, result.sym_cnt + 1);

	if (result.data) {
		size_t offset = 0;
		for (size_t i = 0; i < s1.sym_cnt; i++) result.data[offset++] = s1.data[i];
		for (size_t i = 0; i < s2.sym_cnt; i++) result.data[offset++] = s2.data[i];
		result.data[offset] = L'\0';
	}
	return result;
}

unsigned char GetRussianCharIndex(const char* utf8_bytes, size_t* out_char_size) {
	unsigned char b1 = (unsigned char)utf8_bytes[0];
	unsigned char is_russian = ((b1 & 0xFE) == 0xD0);
	*out_char_size = 1 + is_russian;

	unsigned char b2 = (unsigned char)utf8_bytes[is_russian];
	unsigned char is_d1 = b1 & 1;

		unsigned char b2_low = b2 | (0x20 >> is_d1);

				unsigned char raw_index = (unsigned char)((b2_low - 0xB0) + (is_d1 * 48));

		unsigned char is_yo = (is_d1 & (b2_low == 0x91)) | ((!is_d1) & (b2_low == 0xA1));
	unsigned char yo_mask = -is_yo;

		raw_index = (raw_index & ~yo_mask) | (5 & yo_mask);

		unsigned char is_valid = is_russian & (raw_index < 32);

		return (raw_index & -is_valid) | (255 & ~- is_valid);
}
