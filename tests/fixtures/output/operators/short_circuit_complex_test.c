// 複雜短路求值測試
int printf(char *format, ...);

int main() {
    int count_a = 0;
    int count_b = 0;
    
    // (0 && (count_a = count_a + 1)) || (count_b = count_b + 1)
    // 第一部分短路不執行 count_a++，返回 0
    // 然後 0 || (count_b = count_b + 1) 執行 count_b++
    int result = (0 && (count_a = count_a + 1)) || (count_b = count_b + 1);
    
    // count_a = 0, count_b = 1
    printf("%d\n", count_a * 10 + count_b);
    return 0;  // 預期輸出: 1
}
