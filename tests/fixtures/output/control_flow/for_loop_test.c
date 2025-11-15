// for 迴圈測試
int printf(char *format, ...);

int main() {
    int sum = 0;
    for (int i = 1; i <= 5; i++) {
        sum = sum + i;
    }
    printf("%d\n", sum);
    return 0;  // 應該返回 15 (1+2+3+4+5)
}