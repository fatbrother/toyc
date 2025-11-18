// Test 8: Comma in condition

int printf(char *format, ...);

int main() {
    int result = 0;
    int cond_x = 5;
    int cond_y = 10;
    if ((cond_x = cond_x + 1, cond_y = cond_y - 1, cond_x < cond_y)) {
        result = result + 10;  // +10 (condition is true: 6 < 9)
    }
    
    printf("%d\n", result);  // Expected: 10
    return 0;
}
