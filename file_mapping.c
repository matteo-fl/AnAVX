#include "file_mapping.h"
#include "windows.h"
int Map_OpenDictFile(DictFile* out_file, const wchar_t* path) {
	HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		return 0;
	}
	LARGE_INTEGER size;
	if (!GetFileSizeEx(file, &size)) {
		CloseHandle(file);
		return 0;
	}
	out_file->size = size.QuadPart;
	HANDLE file_map = CreateFileMappingW(file, NULL, PAGE_READONLY, 0, 0, NULL);
	if (file_map == NULL) {
		CloseHandle(file);
		return 0;
	}
	out_file->data = (const char*)MapViewOfFile(file_map, FILE_MAP_READ, 0, 0, 0);
	if (out_file->data == NULL) {
		CloseHandle(file_map);
		CloseHandle(file);
		return 0;
	}

	out_file->_file_handle = (void*)file;
	out_file->_mapping_handle = (void*)file_map;
	return 1;
}

void Map_CloseDictFile(DictFile* file) {
	UnmapViewOfFile(file->data);
	CloseHandle((HANDLE)file->_mapping_handle);
	CloseHandle((HANDLE)file->_file_handle);

	file->data = NULL;
	file->size = 0;
	file->_file_handle = NULL;
	file->_mapping_handle = NULL;
}
