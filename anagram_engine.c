#include "anagram_engine.h"
#include <windows.h>
#include <immintrin.h>
#include "arena.h"
#include "string.h"

void Anagram_BuildQuery(const stringW* input, unsigned int* out_mask, LetterFreqVector* out_freq, unsigned char* out_char_cnt) {
	*out_mask = 0;
	*out_char_cnt = 0;
	memset(out_freq->counts, 0, 32);
	for (size_t i = 0; i < input->sym_cnt; i++) {
		wchar_t c = input->data[i];

		if (c >= 0x0410 && c <= 0x042F) {
			c += 0x0020;
		}

		if (c == 0x0401 || c == 0x0451) {
			c = 0x0435;
		}

		int idx = (int)(c - 0x0430);

		if (idx >= 0 && idx < 32) {
			*out_mask |= (1 << idx);                   
			out_freq->counts[idx]++;                   
			(*out_char_cnt)++;
		}
	}
}



void Anagram_Search(
	const WordIndexEntry* index_table,
	const LetterFreqVector* freq_table,
	size_t word_count,
	unsigned int query_mask,
	const LetterFreqVector* query_freq,
	unsigned char query_char_cnt,
	Arena* temp_arena)
{
	__m256i v_query = _mm256_load_si256((const __m256i*)query_freq->counts);

	for (size_t i = 0; i < word_count; i++) {

		if ((index_table[i].letter_mask & ~query_mask) != 0) {
			continue;
		}

		if (index_table[i].length_chars > query_char_cnt) {
			continue;
		}

		__m256i v_dict = _mm256_load_si256((const __m256i*) & freq_table[i].counts);

		__m256i v_diff = _mm256_subs_epu8(v_dict, v_query);

		if (_mm256_testz_si256(v_diff, v_diff)) {

			ArenaMarker temp_marker = ArenaGetMarker(temp_arena);

			size_t wbuf_chars = index_table[i].length_bytes + 1;
			wchar_t* wbuf = (wchar_t*)ArenaPushArray(temp_arena, wchar_t, wbuf_chars);
			
			if (wbuf != NULL) {
				
				int wlen = MultiByteToWideChar(
					CP_UTF8,
					0,
					index_table[i].utf8_ptr,
					index_table[i].length_bytes,
					wbuf,
					(int)wbuf_chars 
				);

				if (wlen > 0) {
					stringW found_word;
					found_word.data = wbuf;
					found_word.sym_cnt = (size_t)wlen;

					PrintStringW(found_word);
					PrintStringW(C_STR("\n"));
				}
			}
			

			ArenaResetToMarker(temp_arena, temp_marker);
		}
	}
}
