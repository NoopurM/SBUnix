#include <sys/memcpy.h>

void memcpy(void *dest, void  *src, uint32_t len) {
    unsigned char *d = (unsigned char *)dest;
    unsigned char *s = (unsigned char *)src;
    if ((s < d) && (s+len >  d)) {
        d += len;
        s += len;
        for(; len>0; len--) {
            *--d = *--s;
        }
    } else {
        for(int i=0;i<len;i++) {
            d[i] = s[i];
        }
    }
}
