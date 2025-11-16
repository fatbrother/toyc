// Test 11: Order of evaluation test

int printf(char *format, ...);

int main() {
    int order = 0;
    int eval = ((order = 1), (order = 2), (order = 3));  // eval = 3, order = 3
    
    printf("%d\n", eval + order);  // Expected: 6 (3 + 3)
    return 0;
}
