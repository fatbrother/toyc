// 三元運算符測試
int main() {
    int a = 10;
    int b = 5;
    
    // 基本三元運算符
    int max = (a > b) ? a : b;          // 10
    int min = (a < b) ? a : b;          // 5
    
    // 巢狀三元運算符
    int c = 15;
    int largest = (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c);  // 15
    
    // 用於賦值
    int sign = (a > 0) ? 1 : (a < 0) ? -1 : 0;  // 1
    
    return 0;
}