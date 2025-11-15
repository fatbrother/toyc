// AND 不短路測試（兩個操作數都會被評估）
int printf(char *format, ...);

int main() {
    int counter = 0;

    // AND 不短路: 第一個條件為真，第二個會被評估
    int result = 1 && (counter = counter + 1);

    // counter 應該是 1，因為右側被求值了
    printf("%d\n", counter);
    return 0;  // 預期輸出: 1
}
