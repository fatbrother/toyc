// 三元運算子輸出測試
int printf(char *format, ...);

int main() {
    int a = 10;
    int b = 5;

    // 基本三元運算
    int max = (a > b) ? a : b;

    printf("%d\n", max);
    return 0;  // 預期輸出: 10
}
