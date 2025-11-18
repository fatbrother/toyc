// Test 4: Multiple evaluations with comma

int printf(char *format, ...);

int main() {
    int count = 0;
    int val3 = (count = count + 1, count = count + 1, count = count + 1, count);
    
    printf("%d\n", val3);  // Expected: 3 (count is 3)
    return 0;
}
