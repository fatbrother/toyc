// Test 1: Basic comma operator - returns rightmost value

int printf(char *format, ...);

int main() {
    int x, y, z;
    int val1 = (x = 1, y = 2, z = 3);  // val1 = 3
    
    printf("%d\n", val1);  // Expected: 3
    return 0;
}
