// 函數參數測試
int no_params() {
    return 42;
}

int one_param(int x) {
    return x * 2;
}

int multiple_params(int a, int b, int c) {
    return a + b + c;
}

int mixed_types(int i, float f, char c) {
    return i + f;
}

void void_params(void) {
    return;
}

int main() {
    int result1 = no_params();
    int result2 = one_param(10);
    int result3 = multiple_params(1, 2, 3);
    int result4 = mixed_types(5, 2.5, 'A');
    
    void_params();
    
    return 0;
}