#include <unistd.h>
#include <string.h>

void printf(char *str) {
    write(1, str, strlen(str)+1);
}
