// Test 12: Comma returns rightmost value

int printf(char *format, ...);

int main() {
    int multi = (10, 20, 30, 40, 50);  // multi = 50
    
    printf("%d\n", multi);  // Expected: 50
    return 0;
}
