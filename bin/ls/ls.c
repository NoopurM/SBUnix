#include <unistd.h>
int main(int argc, char *argv[], char *envp[]) {
    //write(1, "In ls", 5);
    listf(argv[2]);
    //write(1, "After listf", 11);
    return 0;
}
