int printf(char *format, ...);

int main() {
    int result = 0;
    int x, y, z;
    
    // Test 1: Basic comma operator
    int val1 = (x = 1, y = 2, z = 3);  // val1 = 3
    printf("val1=%d, x=%d, y=%d, z=%d\n", val1, x, y, z);
    result = result + val1;  // +3
    result = result + x + y + z;  // +6
    printf("After Test 1: result=%d (should be 9)\n", result);
    
    return 0;
}
