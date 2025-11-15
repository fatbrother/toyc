// 遞歸函數測試
int printf(char *format, ...);

int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

int main() {
    printf("%d\n", factorial(5));
    return 0;  // 應該返回 120
}