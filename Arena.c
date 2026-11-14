#include "arena.h"

int ArenaInit(Arena* arena, size_t reserve_size) {
	if (arena == NULL) {
		return 0;
	}
	arena->_base = (unsigned char*)os_reserve(reserve_size);
	if (arena->_base == NULL) return 0;

	size_t initial_commit = 64 * 1024;
	if (os_commit(arena->_base, initial_commit) != 1) {
		os_release(arena->_base, reserve_size);
		return 0;
	}

	arena->_capacity = reserve_size;
	arena->_committed = initial_commit;
	arena->_used = 0;
	return 1;
}

void* ArenaPush(Arena* arena, size_t size) {
	if (arena == NULL || size == 0) {
		return NULL;
	}

	size_t aligned_size = ALIGN_UP(size, 32);

	if (arena->_used + aligned_size > arena->_committed) {

		size_t bytes_needed = (arena->_used + aligned_size) - arena->_committed;

		size_t chunk_size = 4 * 1024 * 1024;
		size_t size_to_commit = ALIGN_UP(bytes_needed, chunk_size);

		if (arena->_committed + size_to_commit > arena->_capacity) {
			return NULL;
		}

		if (!os_commit(arena->_base + arena->_committed, size_to_commit)) {
			return NULL;
		}

		arena->_committed += size_to_commit;
	}

	void* result = arena->_base + arena->_used;

	arena->_used += aligned_size;

	return result;
}

void ArenaDestroy(Arena* arena) {
	if (arena == NULL || arena->_base == NULL) {
		return;
	}
	os_release(arena->_base, arena->_capacity);
	arena->_base = NULL;
	arena->_capacity = 0;
	arena->_committed = 0;
	arena->_used = 0;
}

ArenaMarker ArenaGetMarker(Arena* arena) {
	ArenaMarker marker = { 0 };
	if (arena != NULL) {
		marker.used = arena->_used;
		marker.committed = arena->_committed;
	}
	return marker;
}

void ArenaResetToMarker(Arena* arena, ArenaMarker marker) {
	if (arena == NULL) return;
	if (marker.used > arena->_capacity) return;

	arena->_used = marker.used;
}