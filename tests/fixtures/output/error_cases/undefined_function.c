// 語法錯誤：未定義的函數
int printf(char *format, ...);

int main() {
    int result = undefined_function(10, 5);
    printf("%d\n", result);
    return 0;
}