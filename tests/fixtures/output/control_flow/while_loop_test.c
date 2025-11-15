// while 迴圈測試
int printf(char *format, ...);

int main() {
    int count = 0;
    int i = 1;
    while (i <= 4) {
        count = count + i;
        i++;
    }
    printf("%d\n", count);
    return 0;  // 應該返回 10 (1+2+3+4)
}