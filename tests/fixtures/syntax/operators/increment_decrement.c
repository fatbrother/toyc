// 遞增遞減運算符測試
int main() {
    int a = 5;
    int b = 5;
    
    // 前置遞增/遞減
    int pre_inc = ++a;      // a = 6, pre_inc = 6
    int pre_dec = --b;      // b = 4, pre_dec = 4
    
    int c = 5;
    int d = 5;
    
    // 後置遞增/遞減
    int post_inc = c++;     // post_inc = 5, c = 6
    int post_dec = d--;     // post_dec = 5, d = 4
    
    return 0;
}