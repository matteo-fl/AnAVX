#pragma once
#include <stddef.h>
#include <stdint.h>
#include "arena.h"
#include "string8.h"

typedef struct WordIndexEntry {
	const char* utf8_ptr;
	uint32_t letter_mask;
	uint16_t length_bytes;
	uint8_t  length_chars;
	uint8_t  _pad;
} WordIndexEntry;

typedef struct LetterFreqVector {
	uint8_t counts[32];
} LetterFreqVector;

typedef struct AnagramResult {
	string8* words;
	size_t count;
} AnagramResult;

void Anagram_BuildQuery(
	const string8* input,
	unsigned int* out_mask,
	LetterFreqVector* out_freq,
	unsigned char* out_char_cnt
);

AnagramResult Anagram_Search(
	const WordIndexEntry* index_table,
	const LetterFreqVector* freq_table,
	size_t word_count,
	unsigned int query_mask,
	const LetterFreqVector* query_freq,
	unsigned char query_char_cnt,
	Arena* temp_arena
);
