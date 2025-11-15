// 多個標籤的 goto
int printf(char *format, ...);

int main() {
    int x = 1;

    goto label2;

label1:
    x = x + 10;
    goto label3;

label2:
    x = x + 100;
    goto label1;

label3:
    printf("%d\n", x);
    return 0;  // 預期輸出: 111 (1 + 100 + 10)
}
