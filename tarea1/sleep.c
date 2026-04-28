#include <stdio.h>
#include <unistd.h>

int main() {
    for (int i = 1; i <= 5; i++) {
        sleep(10);
        printf("Iteración %d de 5 completada\n", i);
        fflush(stdout);
    }
    return 0;
}
