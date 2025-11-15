// if-else 控制流測試
int printf(char *format, ...);

int main() {
    int x = 10, res = 0;
    if (x > 5) {
        res = 1;  // 應該執行這裡
    } else {
        res = 0;
    }

    printf("%d\n", res);
    return 0;  // 應該返回 1
}