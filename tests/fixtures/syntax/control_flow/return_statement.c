// return 語句測試
int calculate_sum(int n) {
    int sum = 0;
    int i;
    
    for (i = 1; i <= n; i++) {
        sum = sum + i;
    }
    
    return sum;  // 返回計算結果
}

int check_positive(int x) {
    if (x > 0) {
        return 1;  // 提早返回
    }
    
    return 0;
}

void print_message() {
    return;  // void 函數的 return
}

int main() {
    int result = calculate_sum(10);
    int is_positive = check_positive(5);
    
    print_message();
    
    return 0;  // main 函數的 return
}