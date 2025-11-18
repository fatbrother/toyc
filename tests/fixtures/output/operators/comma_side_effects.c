// Test 2: Comma with side effects

int printf(char *format, ...);

int main() {
    int a = 5;
    int b = 10;
    int val2 = (a = a + 1, b = b * 2, a + b);  // a=6, b=20, val2=26
    
    printf("%d\n", val2);  // Expected: 26
    return 0;
}
