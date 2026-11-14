#pragma once
#include <stddef.h>
#include <stdint.h>
#include "platform.h"

#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))
#define ArenaPushArray(arena, type, count) ((type*)ArenaPush((arena), sizeof(type) * (count)))

typedef struct Arena {
	unsigned char* _base;
	size_t _capacity;
	size_t _committed;
	size_t _used;
} Arena;

typedef struct ArenaMarker {
	size_t used;
	size_t committed;
} ArenaMarker;

int         ArenaInit(Arena* arena, size_t reserve_size);
void* ArenaPush(Arena* arena, size_t size);
void        ArenaDestroy(Arena* arena);
ArenaMarker ArenaGetMarker(Arena* arena);
void        ArenaResetToMarker(Arena* arena, ArenaMarker marker);
