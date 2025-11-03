// OR 短路測試
int main() {
    int counter = 0;
    
    // OR 短路: 第一個條件為真，不應執行第二個
    int result = 1 || (counter = counter + 1);
    
    // counter 應該還是 0，因為右側沒有被求值
    return counter;  // 預期輸出: 0
}
