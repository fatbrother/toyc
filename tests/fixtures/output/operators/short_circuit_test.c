// 邏輯運算短路求值測試
int global_counter = 0;

int increment_and_return() {
    global_counter = global_counter + 1;
    return 1;
}

int main() {
    int result;

    // AND 短路: 第一個條件為假，不應執行第二個
    result = 0 && increment_and_return();  // global_counter 應該還是 0

    // OR 短路: 第一個條件為真，不應執行第二個
    result = 1 || increment_and_return();  // global_counter 應該還是 0

    return global_counter;  // 預期輸出: 0
}
