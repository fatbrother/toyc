// 複合賦值運算子測試
int printf(char *format, ...);

int main() {
    int x = 10;

    x = x + 5;     // 15
    x = x - 3;     // 12
    x = x * 2;     // 24
    x = x / 4;     // 6
    x = x + 4;     // 10

    printf("%d\n", x);
    return 0;  // 預期輸出: 10
}
