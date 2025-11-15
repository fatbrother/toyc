// 簡單函數調用
int printf(char *format, ...);

int add(int a, int b) {
    return a + b;
}

int main() {
    int result = add(10, 5);
    printf("%d\n", result);
    return 0;  // 應該返回 15
}