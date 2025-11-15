// 簡單的 goto 測試
int printf(char *format, ...);

int main() {
    int x = 0;
    goto skip;
    x = 10;  // 不會執行
skip:
    x = x + 5;
    printf("%d\n", x);
    return 0;  // 預期輸出: 5
}
