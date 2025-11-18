// Test 6: Nested comma expressions

int printf(char *format, ...);

int main() {
    int x, y, z;
    int m = (x = 1, (y = 2, z = 3), x + y + z);  // m = 6
    
    printf("%d\n", m);  // Expected: 6
    return 0;
}
