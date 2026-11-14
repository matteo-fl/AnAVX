
#include "arena.h"
#include "console_io.h"
#include "anagram_engine.h"

void mainCRTStartup(void) {
	SetConsoleCP(65001);
	SetConsoleOutputCP(65001);

	/*LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	INT64 clock_frequency = freq.QuadPart;*/
	int64_t clock_frequency = 0;
	QueryPerformanceFrequency(&clock_frequency);


	Arena arena;
	ArenaInit(&arena, 256 * 1024 * 1024);

	DictFile dfile;
	if ((&dfile, L"russian.txt")) {

		//LARGE_INTEGER t_start, t_end;
		int64_t t_start = 0, t_end = 0;
		QueryPerformanceCounter(&t_start);


		WordIndexEntry* index_table = ArenaPushArray(&arena, WordIndexEntry, 1500000);
		LetterFreqVector* freq_table = ArenaPushArray(&arena, LetterFreqVector, 1500000);
		size_t word_cnt = 0;

		const char* file_end = dfile.data + dfile.size;
		const char* line_start = dfile.data;
		const char* line_end = line_start;

		while (line_end < file_end && word_cnt < 1500000) {
			line_start = line_end;
			while (line_end < file_end && *line_end != '\n' && *line_end != '\r') {
				line_end++;
			}
			size_t len_bytes = line_end - line_start;
			const char* scan = line_start;
			size_t char_size = 0;

			unsigned int local_mask = 0;
			unsigned char char_cnt = 0;
			unsigned char local_counts[32];
			memset(local_counts, 0, 32);
			int is_word_valid = 1;
			while (scan < line_end) {
				unsigned char idx = GetRussianCharIndex(scan, &char_size);
				if (idx != 255) {
					local_mask |= (1 << idx);
					local_counts[idx]++;
					char_cnt++;
				}
				else {
					is_word_valid = 0;
				}
				scan += char_size;
			}
			if (is_word_valid && char_cnt > 0) {
				index_table[word_cnt].letter_mask = local_mask;
				index_table[word_cnt].utf8_ptr = line_start;
				index_table[word_cnt].length_bytes = (unsigned short)len_bytes;
				index_table[word_cnt].length_chars = char_cnt;
			}
			memcpy(freq_table[word_cnt].counts, local_counts, 32);
			word_cnt++;

			while (line_end < file_end && (*line_end == '\n' || *line_end == '\r')) {
				line_end++;
			}
		}

		QueryPerformanceCounter(&t_end);

		//INT64 parse_ns = ((t_end.QuadPart - t_start.QuadPart) * 1000000000) / clock_frequency;
		int64_t parse_ns = ((t_end - t_start) * 1000000000) / clock_frequency;

		PrintStringW(C_STR("Словарь загружен. Обработано слов: "));
		PrintInt(word_cnt);
		PrintStringW(C_STR("\nВремя парсинга словаря: "));
		PrintInt(parse_ns);
		PrintStringW(C_STR(" нс ("));
		PrintInt(parse_ns / 1000000);         PrintStringW(C_STR(" мс)\n\n"));

		ArenaMarker runtime_start = ArenaGetMarker(&arena);

		while (1) {
			PrintStringW(C_STR("Введите слово для поиска анаграмм: "));
			string8 user_input = InputString8(&arena);

			unsigned int q_mask;
			LetterFreqVector q_freq;
			unsigned char q_char_cnt;

			Anagram_BuildQuery(&user_input, &q_mask, &q_freq, &q_char_cnt);

			PrintStringW(C_STR("\nРезультаты поиска:\n"));

			//LARGE_INTEGER s_start, s_end;
			int64_t s_start = 0, s_end = 0;
			QueryPerformanceCounter(&s_start);

			Anagram_Search(index_table, freq_table, word_cnt, q_mask, &q_freq, q_char_cnt, &arena);

			QueryPerformanceCounter(&s_end);

			//INT64 search_ns = ((s_end.QuadPart - s_start.QuadPart) * 1000000000) / clock_frequency;
			int64_t search_ns = ((s_end - s_start) * 1000000000) / clock_frequency;

			PrintStringW(C_STR("----------------------------------------\n"));
			PrintStringW(C_STR("Время работы AVX2: "));
			PrintInt(search_ns);
			PrintStringW(C_STR(" нс ("));

			PrintInt(search_ns / 1000);
			PrintStringW(C_STR(" мкс)\n\n"));

			ArenaResetToMarker(&arena, runtime_start);
		}

		Map_CloseDictFile(&dfile);
	}
	Pause();
	ArenaDestroy(&arena);
	ExitProcess(0);
}

