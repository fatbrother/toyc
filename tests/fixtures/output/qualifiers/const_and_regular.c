int printf(char *format, ...);

int main() {
    const int limit = 10;
    int sum = 0;
    int i = 0;
    while (i < limit) {
        sum = sum + i;
        i = i + 1;
    }
    printf("%d\n", sum);
    return 0;
}
