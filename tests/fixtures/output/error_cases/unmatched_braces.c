// 語法錯誤：不匹配的大括號
int printf(char *format, ...);

int main() {
    int x = 10;
    if (x > 5) {
        return 1;
    // 缺少閉合大括號
    printf("%d\n", 0);
    return 0;
}