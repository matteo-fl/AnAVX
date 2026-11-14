#pragma once
#include <stddef.h>
#include "platform.h"

typedef struct DictFile {
	const char* data;
	size_t size;
	OsMappedFile _os_map;
} DictFile;

// Возвращает 1 при успехе, 0 при ошибке (сохранена ваша оригинальная логика возврата)
static inline int Map_OpenDictFile(DictFile* out_file, const char* path) {
	if (!out_file || !path) return 0;
	*out_file = (DictFile){ 0 };

	if (os_map_file_readonly(&out_file->_os_map, path) != 0) {
		return 0;
	}

	out_file->data = out_file->_os_map.data;
	out_file->size = out_file->_os_map.size;
	return 1;
}

static inline void Map_CloseDictFile(DictFile* file) {
	if (!file) return;
	os_unmap_file(&file->_os_map);
	*file = (DictFile){ 0 };
}
