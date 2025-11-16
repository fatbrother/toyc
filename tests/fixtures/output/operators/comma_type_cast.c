// Test 7: Comma with different types

int printf(char *format, ...);

int main() {
    float f;
    int n = (f = 3.14, (int)f);  // n = 3
    
    printf("%d\n", n);  // Expected: 3
    return 0;
}
