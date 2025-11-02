// continue 語句輸出測試
int main() {
    int i;
    int sum = 0;

    // 測試 for 循環中的 continue
    for (i = 0; i < 10; i++) {
        if (i == 5) {
            continue;
        }
        sum = sum + i;
    }

    return sum;  // 預期輸出: 40 (0+1+2+3+4+6+7+8+9)
}
