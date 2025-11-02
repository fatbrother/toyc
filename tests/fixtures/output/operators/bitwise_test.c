// 位元運算測試
int main() {
    int a = 12;  // 1100 in binary
    int b = 10;  // 1010 in binary

    int and_result = a & b;   // 1000 = 8
    int or_result = a | b;    // 1110 = 14
    int xor_result = a ^ b;   // 0110 = 6

    int result = and_result + or_result + xor_result;

    return result;  // 預期輸出: 28 (8 + 14 + 6)
}
