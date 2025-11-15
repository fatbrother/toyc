// 複雜算術運算
int printf(char *format, ...);

int main() {
    int a = 10;
    int b = 3;
    int c = 2;
    int result = a + b * c - 1;  // 10 + 6 - 1 = 15
    printf("%d\n", result);
    return 0;
}