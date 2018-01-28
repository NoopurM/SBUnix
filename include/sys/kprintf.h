#ifndef __KPRINTF_H
#define __KPRINTF_H

void kprintf(const char *fmt, ...);
void print_timer(int hr, int mins, int secs);
void print_glyph(char c);
void change_video_ptr();
void clear_screen();
#endif
