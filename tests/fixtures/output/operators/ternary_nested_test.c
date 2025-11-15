// 巢狀三元運算子測試
int printf(char *format, ...);

int main() {
    int a = 10;
    int b = 5;
    int c = 15;

    // 找出最大值
    int largest = (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c);

    printf("%d\n", largest);
    return 0;  // 預期輸出: 15
}
