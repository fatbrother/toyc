// 巢狀循環中的 break 和 continue 測試
int printf(char *format, ...);

int main() {
    int i;
    int j;
    int sum = 0;

    // 測試巢狀循環中的 break 和 continue
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 10; j++) {
            if (j < 3) {
                continue;  // 跳過前三個
            }
            if (j >= 7) {
                break;  // 大於等於 7 時跳出
            }
            sum = sum + j;  // 累加 3, 4, 5, 6
        }
    }

    printf("%d\n", sum);
    return 0;  // 預期輸出: 90 (5 次循環 * (3+4+5+6) = 5 * 18 = 90)
}
