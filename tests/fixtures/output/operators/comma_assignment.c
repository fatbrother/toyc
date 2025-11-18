// Test 5: Comma in assignment

int printf(char *format, ...);

int main() {
    int p, q;
    int r = (p = 5, q = 10, p + q);  // r = 15
    
    printf("%d\n", r);  // Expected: 15
    return 0;
}
