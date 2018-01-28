#include "string.h"

size_t strlen(const char *s) {
    int i=0;
    if(*s) {
        while(s[i] != '\0') {
            i++;
        }
        return i;
    }
    else
        return 0;
}

int strcmp(const char *s1, const char *s2) {
    int i=0, len1=0, len2=0;
    len1 = strlen(s1);
    len2 = strlen(s2);
    if(len1 != len2) {
        return -1;
    }
    else {
        for(i=0;i<len1;i++) {
            if(s1[i] != s2[i]) {
                break;
            }
        }
    }
    if(i==len1)
        return 0;
    else
        return -1;
}

char *strcat(char *dest, const char *src) {
    int len1=0, i=0,j=0;
    len1 = strlen(dest);
    j=len1;
    while(src[i]!='\0') {
        dest[j++]=src[i++];
    }
    dest[j]='\0';
    return dest;
}

void strcpy(char *src, char *dest) {
    while(*src) {
        *dest = *src;
        src++;
        dest++;
    }
    *dest = '\0';
}
/*int main()
{
    int ret;
    char *s1 = {"He  abcde"};
    char *s2 = {"He  abcde"};
    char *s3;
    ret = strlen(s1);

    if(strcmp(s1,s2)==0) {
        printf("same\n");
    }
    else
        printf("not same\n");

    s3 = strcat(s1,s2);
 //   printf("%s",s3);
    return 0;
}*/
