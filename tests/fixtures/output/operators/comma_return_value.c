// Test 9: Return with comma

int printf(char *format, ...);

int main() {
    int final = (1, 2, 3, 4, 5);  // final = 5
    
    printf("%d\n", final);  // Expected: 5
    return 0;
}
