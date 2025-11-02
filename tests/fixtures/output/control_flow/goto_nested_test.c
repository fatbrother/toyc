// goto 跳出巢狀循環
int main() {
    int i;
    int j;
    int result = 0;

    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            if (i == 3 && j == 5) {
                result = i * 10 + j;
                goto found;
            }
        }
    }

found:
    return result;  // 預期輸出: 35 (3*10 + 5)
}
