// 函數調用測試
int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    
    return n * factorial(n - 1);  // 遞歸調用
}

int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    
    return fibonacci(n - 1) + fibonacci(n - 2);  // 多重遞歸調用
}

int helper_function(int x) {
    return x * 2;
}

int complex_calculation(int a, int b) {
    int temp = helper_function(a);  // 調用其他函數
    return temp + b;
}

int main() {
    int fact = factorial(5);
    int fib = fibonacci(6);
    int result = complex_calculation(10, 5);
    
    // 巢狀函數調用
    int nested = factorial(helper_function(3));
    
    return 0;
}