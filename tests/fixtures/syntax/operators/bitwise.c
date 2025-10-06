// 位元運算符測試
int main() {
    int a = 12;     // 1100 in binary
    int b = 10;     // 1010 in binary
    
    int bit_and = a & b;    // 8 (1000)
    int bit_or = a | b;     // 14 (1110)
    int bit_xor = a ^ b;    // 6 (0110)
    int bit_not = ~a;       // complement of 12
    
    return 0;
}