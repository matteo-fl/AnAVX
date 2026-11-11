#pragma once
#include <stddef.h>

typedef struct DictFile
{
	const char* data;
	size_t size;

	void* _file_handle;    
	void* _mapping_handle; 
} DictFile;


int Map_OpenDictFile(DictFile* out_file, const wchar_t* path);

void Map_CloseDictFile(DictFile* file);