// 比較運算符測試
int main() {
    int a = 10;
    int b = 5;
    
    int lt = a < b;         // 0 (false)
    int le = a <= b;        // 0 (false)
    int gt = a > b;         // 1 (true)
    int ge = a >= b;        // 1 (true)
    int eq = a == b;        // 0 (false)
    int ne = a != b;        // 1 (true)
    
    return 0;
}