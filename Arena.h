#pragma once

#define ALIGN_UP(size, align) (((size) + ((align) - 1)) & ~((align) - 1))
#define ArenaPushType(arena, type) ((type*)ArenaPush((arena), sizeof(type)))
#define ArenaPushArray(arena, type, sym_cnt) ((type*)ArenaPush((arena), sizeof(type) * (sym_cnt)))


typedef struct ArenaBlock {
    struct ArenaBlock* _next;
    size_t _capacity;
    size_t _used;
    unsigned char _data[]; } ArenaBlock;

typedef struct {
    ArenaBlock* _first_block;
    ArenaBlock* _current_block;
    size_t _block_size;
} Arena;

typedef struct {
    ArenaBlock* block;
    size_t used;
} ArenaMarker;

int ArenaInit(Arena* arena, size_t default_block_size);
void* ArenaPush(Arena* arena, size_t size);
void ArenaDestroy(Arena* arena);

ArenaMarker ArenaGetMarker(Arena* arena);
void ArenaResetToMarker(Arena* arena, ArenaMarker marker);
