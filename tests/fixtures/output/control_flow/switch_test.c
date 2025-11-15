// switch-case 語句執行測試
int printf(char *format, ...);

int main() {
    int x = 3;
    int result = 0;

    // 測試基本 switch 功能
    switch (x) {
        case 1:
            result = 10;
            break;
        case 2:
            result = 20;
            break;
        case 3:
            result = 30;
            break;
        case 4:
            result = 40;
            break;
        default:
            result = 99;
            break;
    }

    printf("%d\n", result);
    return 0;  // 應該返回 30
}
