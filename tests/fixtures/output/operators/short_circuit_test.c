// 邏輯運算短路求值測試 - AND 短路
int main() {
    int counter = 0;

    // AND 短路: 第一個條件為假，不應執行第二個
    // 使用三元運算子模擬副作用
    int result = 0 && (counter = counter + 1);

    // counter 應該還是 0，因為右側沒有被求值
    return counter;  // 預期輸出: 0
}
