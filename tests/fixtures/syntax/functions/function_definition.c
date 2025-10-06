// 函數定義和聲明測試
int add(int a, int b);          // 函數聲明
float multiply(float x, float y);
void print_number(int n);

int main() {
    int result = add(5, 3);
    float product = multiply(2.5, 4.0);
    print_number(result);
    
    return 0;
}

// 函數定義
int add(int a, int b) {
    return a + b;
}

float multiply(float x, float y) {
    return x * y;
}

void print_number(int n) {
    return;
}