#include <stdio.h>

void exit() {
    for(;;);  
}

int main() {
    while(1) {
        int a,b;
        printf("input two numbers: ");
        scanf("%d %d", &a, &b);
        printf("%d\n", a*b);
    }
}