// 位址和解參考運算符測試
int main() {
    int value = 42;
    int *ptr = &value;      // 取得 value 的位址
    int deref = *ptr;       // 解參考 ptr，取得 value 的值
    
    // 修改指標指向的值
    *ptr = 100;
    
    // 指標的指標
    int **ptr_to_ptr = &ptr;
    int value2 = **ptr_to_ptr;  // 取得 value 的值
    
    return 0;
}