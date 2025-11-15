int printf(char *format, ...);

int main() {
    int value = 3;
    int left = value << 2; // 3 << 2 = 12
    int right = left >> 1; // 12 >> 1 = 6

    int assign = 5;
    assign <<= 1; // 5 << 1 = 10
    assign >>= 2; // 10 >> 2 = 2

    printf("%d\n", left + right + assign);
    return 0; // 12 + 6 + 2 = 20
}
