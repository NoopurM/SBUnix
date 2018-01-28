#include <unistd.h>

extern int catf(char *arg1);

int main(int argc, char *argv[]) {
    catf(argv[2]);
    return 0;
}
