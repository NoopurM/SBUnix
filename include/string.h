#ifndef _STRING_H
#define _STRING_H

#include <sys/defs.h>
int strcmp(const char *s1, const char *s2);
size_t strlen(const char *s);
char *strcat(char *dest, const char *src);
void strcpy(char *s1, char *s2);
#endif

