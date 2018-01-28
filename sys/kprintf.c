#include <sys/kprintf.h>
#include <stdarg.h>
#include "string.h"

static volatile char *video_addr = (volatile char *)0xB8000;
static int cursor_x = 0;

int itoa(unsigned long long no, char *str, int base) {
    int i=0, j, ret = 0, rem;
	char reverse_str[100];
    if (no == 0) {
        str[0] = '0';
        str[1] = '\0';
       return ret;
    }
    while (no > 0) {
        if ( (no % base) < 10) {
            reverse_str[i++] = '0' + (no % base);
        } else {
            rem = (no % base) - 10;
            reverse_str[i++] = 'a' + rem;
        }
        no = no/base;
    }
    reverse_str[i] = '\0';
    j = i-1;
    for (i = 0; j >= 0; j--) {
        str[i++] = reverse_str[j];
    }
    str[i] = '\0';
    return ret;
}

void print_char(volatile char **video_addr, char c) {
    **video_addr = c; //write character to video memory
    (*video_addr)++; //increment to next byte to put colour
    (*video_addr)++;
    cursor_x += 2;
}
void kprintf(const char *fmt, ...)
{
    //static volatile char *video_addr = (volatile char *)0xB8000;
    va_list ap;
    char *c;
    char str[100] = {0};
    int j = 0, no = 0, ret = 0;
    unsigned long long address;
    const char *tmp = fmt;
    va_start(ap, fmt);
    while (*tmp != '\0') {
        j = 0;
        if(*tmp == '%') {
            tmp++;
            if(*tmp == 'd') {
                no = va_arg(ap, int);
                tmp++;
                ret = itoa(no, str, 10);
                //print variable
                while (str[j] != '\0') {
                    print_char(&video_addr, str[j]);
                    j++;
                };
            }
            else if(*tmp == 'x' || *tmp == 'p') {
                if(*tmp == 'p') {
                    print_char(&video_addr, '0');
                    print_char(&video_addr, 'x');
                    }
        
                address = va_arg(ap, unsigned long long);
                tmp++;
                ret = itoa(address, str, 16);
                //print variable
                while (str[j] != '\0') {
                    print_char(&video_addr, str[j]);
                    j++;
                };
            }
            else if(*tmp == 'c') {
               tmp++;
                no = va_arg(ap, int);
                print_char(&video_addr, no);
            }
            else if(*tmp == 's') {
                tmp++;
                c = va_arg(ap,char*);
                while(*c != '\0') {
                    if (*c == '\n') {
                        video_addr = video_addr + (160 - (cursor_x));
                        cursor_x = 0;
                    } else {
                        print_char(&video_addr, *c);
                    }
                    c++;
                }
            }
        }
        else if (*tmp == '\n') {
            video_addr = video_addr + (160 - (cursor_x));
            cursor_x = 0;
            tmp++;
        }
        else {
                print_char(&video_addr, *tmp);
                tmp++;
        }
    }
    va_end(ap);
    ret = ret;
    no = no;
}

void print_glyph(char c) {
    video_addr = (volatile char *)(0xffffffff800b8000); 
    int j = 0;
    cursor_x = 0;
    char str[50] = "Glyph Pressed :";
   	while (str[j] != '\0') {
    	print_char(&video_addr, str[j]);
        j++;
    }; 
    *video_addr = c;
    //print_char(&video_addr, c);
}

void print_timer(int hr, int mins, int secs) {
    char str1[10] = {0}, str2[10] = {0}, str3[10] = {0};
    int i = 0;
    cursor_x = 0;
    itoa(hr, str1, 10);
    itoa(mins, str2, 10);
    itoa(secs, str3, 10);
    video_addr = video_addr + 23*160 + 100;
    do {
        print_char(&video_addr, str1[i]);
        i++;
    }while (str1[i] != '\0');
    print_char(&video_addr, ':');
    i = 0;
    do {
        print_char(&video_addr, str2[i]);
        i++;
    } while (str2[i] != '\0');
    print_char(&video_addr, ':');
    if (secs < 10) {
        print_char(&video_addr, '0');
    }
    i = 0;
    do {
        print_char(&video_addr, str3[i]);
        i++;
    }while (str3[i] != '\0');
}

void change_video_ptr() {
    video_addr = (volatile char *)((0xffffffff80000000) | (uint64_t)video_addr);
}

void clear_screen() {
    video_addr = (volatile char *)(0xffffffff800b8000); 
    int i = 0;
    while(i++ < 80*24*2) {
        *video_addr = ' ';
        video_addr++;
        video_addr++;
        i++;
    }
    video_addr = (volatile char *)(0xffffffff800b8000);
    cursor_x = 0;
}
