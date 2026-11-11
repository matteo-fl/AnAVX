#pragma once
#include <stddef.h>
#include "string.h"
#include "anagram_engine.h" #include "console_io.h"      
typedef struct WordIndexEntry {
	unsigned int letter_mask;
	const char* utf8_ptr;
	unsigned short length_bytes;
	unsigned char length_chars;
	unsigned char padding[8];
} WordIndexEntry;


typedef struct __declspec(align(32)) LetterFreqVector {
	unsigned char counts[32];
} LetterFreqVector;


void Anagram_BuildQuery(const stringW* input, unsigned int* out_mask, LetterFreqVector* out_freq, unsigned char* out_char_cnt);

void Anagram_Search(
	const WordIndexEntry* index_table, const LetterFreqVector* freq_table, size_t word_count, unsigned int query_mask, const LetterFreqVector* query_freq, unsigned char query_char_cnt, Arena* temp_arena);
