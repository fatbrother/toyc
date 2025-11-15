int printf(char *format, ...);

int main() {
    // Test 1: Integer array initialization
    int arr1[3] = {10, 20, 30};
    
    // Test 2: Char array initialization  
    char arr2[4] = {'A', 'B', 'C', 'D'};
    
    // Test 3: Float array initialization
    float arr3[2] = {1.5, 2.5};
    
    // Test 4: Scalar initialization (to ensure we didn't break it)
    int x = 100;
    
    // Test results
    int sum = arr1[0] + arr1[1] + arr1[2];  // 60
    int charTest = (int)arr2[0];  // 65 (A)
    int floatTest = (int)(arr3[0] + arr3[1]);  // 4
    
    printf("%d\n", sum + charTest + floatTest + x);
    return 0;  // Should return 229 (60 + 65 + 4 + 100)
}
