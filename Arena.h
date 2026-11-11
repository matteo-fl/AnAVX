#pragma once

#define ALIGN_UP(size, align) (((size) + ((align) - 1)) & ~((align) - 1))
#define ArenaPushType(arena, type) ((type*)ArenaPush((arena), sizeof(type)))
#define ArenaPushArray(arena, type, sym_cnt) ((type*)ArenaPush((arena), sizeof(type) * (sym_cnt)))

typedef struct Arena {
    unsigned char* _base;           
    size_t _capacity;               
    size_t _committed;              
    size_t _used;               
} Arena;

typedef struct ArenaMarker  {
    size_t used;
    size_t committed;
} ArenaMarker;

int ArenaInit(Arena* arena, size_t reserve_size);
void* ArenaPush(Arena* arena, size_t size);
void ArenaDestroy(Arena* arena);

ArenaMarker ArenaGetMarker(Arena* arena);
void ArenaResetToMarker(Arena* arena, ArenaMarker marker);
