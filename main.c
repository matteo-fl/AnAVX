#include <immintrin.h>
#include "platform.h"
#include "arena.h"
#include "string8.h"
#include "console_io.h"
#include "anagram_engine.h"
#include "file_mapping.h"

// Быстрый встроенный декодер русских букв UTF-8:
// 'а'..'я' и 'А'..'Я' -> индекс 0..31, 'ё'/'Ё' -> 5 ('е'), остальные -> 255
static inline unsigned char FastRuIndex(const unsigned char* s, size_t* step) {
	if (s == 0xD0) {
		unsigned char b = s;
		*step = 2;
		if (b >= 0xB0 && b <= 0xBF) return (unsigned char)(b - 0xB0);      // а..п -> 0..15
		if (b >= 0x90 && b <= 0xAF) return (unsigned char)(b - 0x90);      // А..Я -> 0..31
		if (b == 0x81) return 5;                                           // Ё -> е
		return 255;
	}
	if (s == 0xD1) {
		unsigned char b = s;
		*step = 2;
		if (b >= 0x80 && b <= 0x8F) return (unsigned char)(b - 0x80 + 16); // р..я -> 16..31
		if (b == 0x91) return 5;                                           // ё -> е
		return 255;
	}
	*step = 1;
	return 255;
}

int main(void) {
	os_init();

	Arena arena;
	// Резервируем 512 МБ виртуального пространства (физическая RAM тратится только по факту коммита)
	if (!ArenaInit(&arena, 512ULL * 1024 * 1024)) {
		os_exit(1);
	}

	DictFile dfile;
	if (Map_OpenDictFile(&dfile, "D://All_Projects//C_Projects//AnAVX//russian.txt")) {

		// Подсказываем ОС заранее загрузить файл словаря в физическую память
		os_prefetch_memory(dfile.data, dfile.size);

		uint64_t t_start = os_time_ns();

		WordIndexEntry* index_table = ArenaPushArray(&arena, WordIndexEntry, 1500000);
		LetterFreqVector* freq_table = ArenaPushArray(&arena, LetterFreqVector, 1500000);
		if (!index_table || !freq_table) {
			Map_CloseDictFile(&dfile);
			ArenaDestroy(&arena);
			os_exit(1);
		}

		size_t word_cnt = 0;
		const unsigned char* scan = (const unsigned char*)dfile.data;
		const unsigned char* file_end = scan + dfile.size;
		__m256i v_zero = _mm256_setzero_si256();

		while (scan < file_end && word_cnt < 1500000) {
			// Пропускаем переводы строк
			while (scan < file_end && (*scan == '\n' || *scan == '\r')) {
				scan++;
			}
			if (scan >= file_end) {
				break;
			}

			const unsigned char* line_start = scan;
			uint8_t* target_counts = freq_table[word_cnt].counts;

			// Зануляем 32 байта вектора частот напрямую в таблице одной AVX2-инструкцией
			_mm256_store_si256((__m256i*)target_counts, v_zero);

			uint32_t local_mask = 0;
			uint8_t char_cnt = 0;
			int is_word_valid = 1;

			// Однопроходный разбор слова до конца строки
			while (scan < file_end && *scan != '\n' && *scan != '\r') {
				if (is_word_valid && (scan + 1 < file_end)) {
					size_t step = 1;
					unsigned char idx = FastRuIndex(scan, &step);
					if (idx != 255) {
						local_mask |= (1U << idx);
						target_counts[idx]++;
						char_cnt++;
						scan += step;
						continue;
					}
				}
				is_word_valid = 0;
				scan++;
			}

			if (is_word_valid && char_cnt > 0) {
				index_table[word_cnt].letter_mask = local_mask;
				index_table[word_cnt].utf8_ptr = (const char*)line_start;
				index_table[word_cnt].length_bytes = (uint16_t)(scan - line_start);
				index_table[word_cnt].length_chars = char_cnt;
				word_cnt++;
			}
		}

		uint64_t t_end = os_time_ns();
		uint64_t parse_ns = t_end - t_start;

		PrintString8(STR8("Словарь загружен. Обработано слов: "));
		PrintInt((int64_t)word_cnt);
		PrintString8(STR8("\nВремя парсинга словаря: "));
		PrintInt((int64_t)parse_ns);
		PrintString8(STR8(" нс ("));
		PrintInt((int64_t)(parse_ns / 1000000));
		PrintString8(STR8(" мс)\n\n"));

		ArenaMarker runtime_start = ArenaGetMarker(&arena);

		while (1) {
			PrintString8(STR8("Введите слово для поиска анаграмм: "));
			string8 user_input = InputString8(&arena);

			// Выход при вводе пустой строки
			if (user_input.size == 0) {
				break;
			}

			unsigned int q_mask = 0;
			LetterFreqVector q_freq = { 0 };
			unsigned char q_char_cnt = 0;

			Anagram_BuildQuery(&user_input, &q_mask, &q_freq, &q_char_cnt);

			PrintString8(STR8("\nРезультаты поиска:\n"));

			uint64_t s_start = os_time_ns();

			AnagramResult res = Anagram_Search(index_table, freq_table, word_cnt, q_mask, &q_freq, q_char_cnt, &arena);

			uint64_t s_end = os_time_ns();
			uint64_t search_ns = s_end - s_start;

			for (size_t i = 0; i < res.count; i++) {
				PrintString8(res.words[i]);
				PrintString8(STR8("\n"));
			}

			PrintString8(STR8("----------------------------------------\n"));
			PrintString8(STR8("Время работы AVX2: "));
			PrintInt((int64_t)search_ns);
			PrintString8(STR8(" нс ("));
			PrintInt((int64_t)(search_ns / 1000));
			PrintString8(STR8(" мкс)\n\n"));

			ArenaResetToMarker(&arena, runtime_start);
		}

		Map_CloseDictFile(&dfile);
	}

	Pause();
	ArenaDestroy(&arena);
	os_exit(0);
	return 0;
}
