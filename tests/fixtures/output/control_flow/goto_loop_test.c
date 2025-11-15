// 使用 goto 實現循環
int printf(char *format, ...);

int main() {
    int sum = 0;
    int i = 0;
loop_start:
    if (i >= 5) {
        goto loop_end;
    }
    sum = sum + i;
    i = i + 1;
    goto loop_start;
loop_end:
    printf("%d\n", sum);
    return 0;  // 預期輸出: 10 (0+1+2+3+4)
}
