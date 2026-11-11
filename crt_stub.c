#include <stddef.h>
#include <intrin.h>

#pragma function(memset, memcpy, memmove, memcmp)

void* __cdecl memset(void* dest, int c, size_t count) {
    __stosb((unsigned char*)dest, (unsigned char)c, count);
    return dest;
}

void* __cdecl memcpy(void* dest, const void* src, size_t count) {
    __movsb((unsigned char*)dest, (const unsigned char*)src, count);
    return dest;
}

void* __cdecl memmove(void* dest, const void* src, size_t count) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;

    if (s < d && d < s + count) {
        d += count;
        s += count;
        while (count--) {
            *--d = *--s;
        }
    }
    else {
        __movsb(d, s, count);
    }
    return dest;
}

int __cdecl memcmp(const void* buf1, const void* buf2, size_t count) {
    const unsigned char* s1 = (const unsigned char*)buf1;
    const unsigned char* s2 = (const unsigned char*)buf2;
    while (count--) {
        if (*s1 != *s2) {
            return (*s1 < *s2) ? -1 : 1;
        }
        s1++;
        s2++;
    }
    return 0;
}
