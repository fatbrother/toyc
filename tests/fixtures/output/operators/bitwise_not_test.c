// 位元 NOT 運算測試
int printf(char *format, ...);

int main() {
    int a = 5;        // 0000...0101
    int b = ~a;       // 1111...1010 (負數的二補數表示)
    int c = ~b;       // 回到 0000...0101

    printf("%d\n", c);
    return 0;  // 預期輸出: 5
}
