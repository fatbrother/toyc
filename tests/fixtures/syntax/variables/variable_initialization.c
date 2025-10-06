// 變數初始化測試
int main() {
    // 基本類型初始化
    int a = 10;
    float b = 3.14;
    char c = 'A';

    // 多個變數初始化
    int x = 1, y = 2, z = 3;
    float p = 1.0, q = 2.0;

    // 指標初始化
    int value = 42;
    int *ptr = &value;

    // 使用表達式初始化
    int sum = a + x;
    int product = y * z;

    // 使用函數調用初始化（如果有函數的話）
    int calculated = sum + product;

    return 0;
}