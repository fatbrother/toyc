// break 語句輸出測試
int printf(char *format, ...);

int main() {
    int i;
    int sum = 0;

    // 測試 for 循環中的 break
    for (i = 0; i < 10; i++) {
        if (i == 5) {
            break;
        }
        sum = sum + i;
    }

    printf("%d\n", sum);
    return 0;  // 預期輸出: 10 (0+1+2+3+4)
}
