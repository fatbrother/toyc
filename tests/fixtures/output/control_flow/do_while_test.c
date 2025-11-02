// do-while 循環輸出測試
int main() {
    int sum = 0;
    int i = 0;

    do {
        sum = sum + i;
        i = i + 1;
    } while (i < 5);

    return sum;  // 預期輸出: 10 (0+1+2+3+4)
}
