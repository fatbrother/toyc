// 複雜表達式測試
int calculate(int x, int y) {
    return x * y + 10;
}

int main() {
    int a = 5;
    int b = 3;
    int c = 2;

    // 複合算術表達式
    int result1 = a + b * c - (a / b);

    // 複合邏輯表達式
    int result2 = (a > b) && (b > c) || (a == 5);

    // 混合運算符優先級
    int result3 = a + b * c > a * b - c;

    // 函數調用在表達式中
    int result4 = calculate(a, b) + calculate(b, c);

    // 指標和解參考在表達式中
    int *ptr = &a;
    int result5 = *ptr + *(ptr) * 2;

    // 三元運算符在複雜表達式中
    int result6 = (a > b) ? (a + b) : (a - b) + (c * 2);

    // 複合賦值與表達式
    a += b * c;
    b *= (a > 10) ? 2 : 1;

    return 0;
}