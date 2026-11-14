#include "string8.h"

string8 String8Literal(const char* text) {
	string8 str = { 0 };
	if (!text) return str;

	size_t len = 0;
	while (text[len] != '\0') {
		len++;
	}

	str.data = (char*)text;
	str.size = len;
	return str;
}

int String8Equals(string8 s1, string8 s2) {
    if (s1.size != s2.size) return 0;
    if (s1.data == s2.data) return 1;

    for (size_t i = 0; i < s1.size; i++) {
        if (s1.data[i] != s2.data[i]) return 0;
    }
    return 1;
}

string8 String8Slice(string8 str, size_t start, size_t end) {
    if (start > str.size) start = str.size;
    if (end > str.size)   end = str.size;
    if (start > end)      start = end;

    string8 result;
    result.data = str.data + start;
    result.size = end - start;
    return result;
}


int String8Concat(void* buffer, size_t buffer_size, string8 s1, string8 s2, string8* result) {
	if (!buffer || !result) {
		return 1;
	}

	if (s1.size >= buffer_size || s2.size > buffer_size - 1 - s1.size) {
		return 1;
	}

	char* dst = (char*)buffer;
	size_t offset = 0;

	for (size_t i = 0; i < s1.size; i++) {
		dst[offset++] = s1.data[i];
	}
	for (size_t i = 0; i < s2.size; i++) {
		dst[offset++] = s2.data[i];
	}
	dst[offset] = '\0';

	result->data = dst;
	result->size = offset;

	return 0;
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

string8 String8FromInt(char* buffer, size_t capacity, int64_t value) {
	string8 result = { 0 };
	if (!buffer || capacity == 0) {
		return result;
	}

	char stack_buf[32];
	size_t char_count = 0;

	int64_t mask = value >> 63;
	uint64_t utemp = (uint64_t)((value ^ mask) - mask);
	int is_negative = (value < 0);

	do {
		uint64_t digit = utemp % 10;
		stack_buf[char_count++] = (char)('0' + digit);
		utemp /= 10;
	} while (utemp > 0 && char_count < 30);

	if (is_negative) {
		stack_buf[char_count++] = '-';
	}

	if (char_count + 1 > capacity) {
		return result;
	}
	for (size_t i = 0; i < char_count; i++) {
		buffer[i] = stack_buf[char_count - 1 - i];
	}
	buffer[char_count] = '\0';

	result.data = buffer;
	result.size = char_count;
	return result;
}

int String8ToInt(string8 str) {
	if (!str.data || str.size == 0) {
		return 0;
	}

	size_t i = 0;

	// 1. Пропускаем начальные пробелы и табуляции
	while (i < str.size && (str.data[i] == ' ' || str.data[i] == '\t' || str.data[i] == '\r' || str.data[i] == '\n')) {
		i++;
	}
	if (i == str.size) {
		return 0;
	}

	// 2. Определяем знак числа
	int is_negative = 0;
	if (str.data[i] == '-') {
		is_negative = 1;
		i++;
	}
	else if (str.data[i] == '+') {
		i++;
	}

	// 3. Собираем цифры в беззнаковый аккумулятор (защита от UB при переполнении int)
	uint32_t acc = 0;
	uint32_t limit = is_negative ? ((uint32_t)INT32_MAX + 1U) : (uint32_t)INT32_MAX;

	while (i < str.size) {
		char c = str.data[i];
		if (c < '0' || c > '9') {
			break; // Останавливаемся на первом нецифровом символе (поведение как у стандартного atoi)
		}

		uint32_t digit = (uint32_t)(c - '0');

		// Проверка переполнения 32-битного int
		if (acc > (limit - digit) / 10U) {
			return is_negative ? INT32_MIN : INT32_MAX;
		}

		acc = acc * 10U + digit;
		i++;
	}

	// 4. Применяем знак
	if (is_negative) {
		if (acc == (uint32_t)INT32_MAX + 1U) {
			return INT32_MIN;
		}
		return -(int)acc;
	}

	return (int)acc;
}
