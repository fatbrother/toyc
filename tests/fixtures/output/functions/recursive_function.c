// 遞歸函數測試
int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

int main() {
    return factorial(5);  // 應該返回 120
}