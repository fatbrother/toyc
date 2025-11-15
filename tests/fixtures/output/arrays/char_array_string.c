int printf(char *format, ...);

int main() {
    char str[6] = {'H', 'e', 'l', 'l', 'o', '\0'};
    // 計算字串長度
    int len = 0;
    while (str[len] != '\0') {
        len = len + 1;
    }
    printf("%d\n", len);
    return 0;  // 應返回 5
}
