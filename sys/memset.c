#include <sys/memset.h>

void *memset(void *s, uint32_t c, uint32_t n)
{
    uint64_t *p = (uint64_t *)s;
    while(n--)
        *p++ = c;
    return s;
}
