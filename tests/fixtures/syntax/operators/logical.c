// 邏輯運算符測試
int main() {
    int a = 1;
    int b = 0;
    
    int and_result = a && b;    // 0 (false)
    int or_result = a || b;     // 1 (true)
    int not_a = !a;             // 0 (false)
    int not_b = !b;             // 1 (true)
    
    // 複合邏輯表達式
    int complex = (a && !b) || (b && !a);  // 1 (true)
    
    return 0;
}