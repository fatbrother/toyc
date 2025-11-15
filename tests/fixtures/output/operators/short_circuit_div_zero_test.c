// 短路求值防止除以零
int printf(char *format, ...);

int main() {
    int x = 10;
    int y = 0;
    int result = 0;

    // 使用短路求值防止除以零錯誤
    // y != 0 為假，所以 x / y 不會被執行
    if (y != 0 && x / y > 2) {
        result = 1;
    } else {
        result = 5;
    }

    printf("%d\n", result);
    return 0;  // 預期輸出: 5
}
