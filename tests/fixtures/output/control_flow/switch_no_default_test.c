// switch 語句測試 - 沒有 default case
int printf(char *format, ...);

int main() {
    int x = 5;
    int result = 100;  // 初始值

    // 測試沒有 default 的 switch
    // x = 5 不匹配任何 case，應該直接跳出
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
    }

    // result 應該保持原值 100
    printf("%d\n", result);
    return 0;  // 預期輸出: 100
}
