#include <windows.h>
#include "arena.h"

static ArenaBlock* CreateBlock(size_t block_size) {
	size_t header_size = ALIGN_UP(sizeof(ArenaBlock), 32);
	size_t total_size = header_size + block_size;

	ArenaBlock* block = (ArenaBlock*)VirtualAlloc(NULL, total_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (block != NULL) {
		block->_capacity = block_size;
		block->_used = 0;
		block->_next = NULL;
	}
	return block;
}

int ArenaInit(Arena* arena, size_t default_block_size) {
	if (arena == NULL) {
		return 0;
	}

	arena->_block_size = default_block_size;
	arena->_first_block = CreateBlock(default_block_size);

	if (arena->_first_block == NULL) {
		return 0;
	}

	arena->_current_block = arena->_first_block;
	return 1;
}

void* ArenaPush(Arena* arena, size_t size) {
	if (arena == NULL || size == 0) {
		return NULL;
	}

	size_t aligned_size = ALIGN_UP(size, 32);

	if (arena->_current_block->_used + aligned_size > arena->_current_block->_capacity) {
		size_t next_size = (aligned_size > arena->_block_size) ? aligned_size : arena->_block_size;

		ArenaBlock* new_block = CreateBlock(next_size);
		if (new_block == NULL) {
			return NULL;
		}

		arena->_current_block->_next = new_block;
		arena->_current_block = new_block;
	}

	void* ptr = &arena->_current_block->_data[arena->_current_block->_used];
	arena->_current_block->_used += aligned_size;

	return ptr;
}

void ArenaDestroy(Arena* arena) {
	if (arena == NULL) {
		return;
	}

	ArenaBlock* current = arena->_first_block;
	while (current != NULL) {
		ArenaBlock* next = current->_next;
		VirtualFree(current, 0, MEM_RELEASE);        
		current = next;
	}

	arena->_first_block = NULL;
	arena->_current_block = NULL;
}

ArenaMarker ArenaGetMarker(Arena* arena) {
	ArenaMarker marker = { 0 };
	if (arena && arena->_current_block) {
		marker.block = arena->_current_block;
		marker.used = arena->_current_block->_used;
	}
	return marker;
}

void ArenaResetToMarker(Arena* arena, ArenaMarker marker) {
	if (arena == NULL || marker.block == NULL) {
		return;
	}

	ArenaBlock* current_to_free = marker.block->_next;

	arena->_current_block = marker.block;
	arena->_current_block->_used = marker.used;
	arena->_current_block->_next = NULL;
	while (current_to_free != NULL) {
		ArenaBlock* next_to_free = current_to_free->_next;
		VirtualFree(current_to_free, 0, MEM_RELEASE);
		current_to_free = next_to_free;
	}
}
