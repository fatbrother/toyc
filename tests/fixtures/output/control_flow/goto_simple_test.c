// 簡單的 goto 測試
int main() {
    int x = 0;
    goto skip;
    x = 10;  // 不會執行
skip:
    x = x + 5;
    return x;  // 預期輸出: 5
}
