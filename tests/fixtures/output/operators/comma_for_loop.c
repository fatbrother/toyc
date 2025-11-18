// Test 3: Comma in for loop

int printf(char *format, ...);

int main() {
    int i, j;
    int sum = 0;
    for (i = 0, j = 10; i < 5; i++, j--) {
        sum = sum + i + j;  // 0+10, 1+9, 2+8, 3+7, 4+6 = 50
    }
    
    printf("%d\n", sum);  // Expected: 50
    return 0;
}
