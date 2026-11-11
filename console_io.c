#include <windows.h>
#include <intrin.h> 
#include "console_io.h"
#include "arena.h"
#include "string.h"

void PrintStringW(stringW str) {
	if (str.sym_cnt == 0 || !str.data) return;
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD written;

	WriteConsoleW(hOut, str.data, (DWORD)str.sym_cnt, &written, NULL);
}

stringW InputStringW(Arena* arena) {
	stringW result = { 0 };
	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);

	size_t maxChars = 65536;

	result.data = ArenaPushArray(arena, wchar_t, maxChars + 1);
	if (!result.data) return result;

	DWORD charsRead = 0;

	if (ReadConsoleW(hIn, result.data, (DWORD)maxChars, &charsRead, NULL)) {
		while (charsRead > 0 && (result.data[charsRead - 1] == L'\r' || result.data[charsRead - 1] == L'\n')) {
			charsRead--;
		}

		result.sym_cnt = charsRead;
		result.data[charsRead] = L'\0';
	}

	return result;
}

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


void PrintInt(INT64 value) {
	wchar_t stack_buf[32] = { 0 };
	size_t char_count = 0;

	INT64 mask = value >> 63;
	UINT64 utemp = (UINT64)((value ^ mask) - mask);
	UINT64 is_negative = (UINT64)(mask & 1);

	do {
		UINT64 next_utemp = utemp / 10;
		UINT64 digit = utemp % 10;

		stack_buf[char_count++] = L'0' + (wchar_t)digit;
		utemp = next_utemp;
	} while (utemp > 0 && char_count < 30);

	if (is_negative) {
		stack_buf[char_count++] = L'-';
	}

	for (size_t i = 0; i < char_count / 2; i++) {
		wchar_t t = stack_buf[i];
		stack_buf[i] = stack_buf[char_count - 1 - i];
		stack_buf[char_count - 1 - i] = t;
	}

	stack_buf[char_count] = L'\0';

	stringW str;
	str.data = stack_buf;
	str.sym_cnt = char_count;
	PrintStringW(str);
}

void AVX_PrintInt(INT64 value) {
	INT64 mask = value >> 63;
	UINT64 utemp = (UINT64)((value ^ mask) - mask);
	UINT32 is_negative = (UINT32)(mask & 1);

	UINT32 val = (UINT32)utemp;

	size_t char_count = 0;
	UINT32 temp = val;
	do {
		char_count++;
		temp /= 10;
	} while (temp > 0);

	__m256i v_divisors = _mm256_set_epi32(10000000, 1000000, 100000, 10000, 1000, 100, 10, 1);
	__m256i v_val = _mm256_set1_epi32(val);

	__m256 v_val_f = _mm256_cvtepi32_ps(v_val);
	__m256 v_div_f = _mm256_cvtepi32_ps(v_divisors);
	__m256 v_res_f = _mm256_div_ps(v_val_f, v_div_f);

	__m256i v_div_res = _mm256_cvttps_epi32(v_res_f);

	__m256i v_shifted = _mm256_permutevar8x32_epi32(v_div_res, _mm256_set_epi32(0, 7, 6, 5, 4, 3, 2, 1));
	v_shifted = _mm256_blend_epi32(v_shifted, _mm256_set1_epi32(0), 0x80);

	__m256i v_mul10 = _mm256_add_epi32(_mm256_slli_epi32(v_shifted, 3), _mm256_slli_epi32(v_shifted, 1));
	__m256i v_digits = _mm256_sub_epi32(v_div_res, v_mul10);

	__m256i v_ascii = _mm256_add_epi32(v_digits, _mm256_set1_epi32(L'0'));

	wchar_t final_buf[32] = { 0 };
	size_t idx = 0;

	if (is_negative) {
		final_buf[idx++] = L'-';
	}

	for (size_t i = 0; i < char_count; i++) {
		int register_lane_idx = (int)(char_count - 1 - i);

		UINT32 ascii_char = 0;
		switch (register_lane_idx) {
		case 0: ascii_char = _mm256_extract_epi32(v_ascii, 0); break;
		case 1: ascii_char = _mm256_extract_epi32(v_ascii, 1); break;
		case 2: ascii_char = _mm256_extract_epi32(v_ascii, 2); break;
		case 3: ascii_char = _mm256_extract_epi32(v_ascii, 3); break;
		case 4: ascii_char = _mm256_extract_epi32(v_ascii, 4); break;
		case 5: ascii_char = _mm256_extract_epi32(v_ascii, 5); break;
		case 6: ascii_char = _mm256_extract_epi32(v_ascii, 6); break;
		case 7: ascii_char = _mm256_extract_epi32(v_ascii, 7); break;
		}
		final_buf[idx++] = (wchar_t)ascii_char;
	}
	final_buf[idx] = L'\0';

	stringW str;
	str.data = final_buf;
	str.sym_cnt = idx;

	PrintStringW(str);
}