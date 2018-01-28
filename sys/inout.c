#include <sys/defs.h>
#include <sys/inout.h>

inline void outb(uint16_t port, uint8_t val)
{
     __asm__ volatile (
                 "outb %0, %1"
                 :
                 : "a"(val), "Nd"(port)
             );
}

inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile(
                "inb %1,%0"
                : "=a"(ret)
                : "Nd"(port)
            );
    return ret;
}

inline void outl(uint16_t port, uint32_t val)
{
     __asm__ volatile (
                 "outl %0, %1"
                 :
                 : "a"(val), "Nd"(port)
             );
}

inline uint32_t inl(uint16_t port) {
     uint32_t ret;
     __asm__ volatile(
                 "inl %1,%0"
                 : "=a"(ret)
                 : "Nd"(port)
             );
     return ret;
}

inline void io_wait(void)
{
    /* TODO: This is probably fragile. */
    __asm__ volatile ( "jmp 1f\n\t"
                   "1:jmp 2f\n\t"
                   "2:" );
}
