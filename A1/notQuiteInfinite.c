#include <stdio.h>
#include <unistd.h>
int main() {
    int i = 0;
    while(i < 60) {
        printf("%d \n", i);
        sleep(10);
        i++;
    }

        return 1;
}
