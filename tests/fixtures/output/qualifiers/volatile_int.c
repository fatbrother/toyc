int printf(char *format, ...);

int main() {
    volatile int x = 5;
    x = x + 1;
    printf("%d\n", x);
    return 0;
}
