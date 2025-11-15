// 最簡單的程式：返回固定值
int printf(char *format, ...);

int test() {
    return 42;
}

int main() {
    printf("%d\n", test());
    return 0;
}