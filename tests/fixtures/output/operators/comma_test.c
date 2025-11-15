// Test comma operator output

int printf(char *format, ...);

int main() {
    int result = 0;
    int x, y, z;
    
    // Test 1: Basic comma operator - returns rightmost value
    int val1 = (x = 1, y = 2, z = 3);  // val1 = 3
    result = result + val1;  // +3
    result = result + x + y + z;  // +6 (1+2+3)
    // result = 9
    
    // Test 2: Comma with side effects
    int a = 5;
    int b = 10;
    int val2 = (a = a + 1, b = b * 2, a + b);  // a=6, b=20, val2=26
    result = result + val2;  // +26
    // result = 35
    
    // Test 3: Comma in for loop
    int i, j;
    int sum = 0;
    for (i = 0, j = 10; i < 5; i++, j--) {
        sum = sum + i + j;  // 0+10, 1+9, 2+8, 3+7, 4+6 = 50
    }
    result = result + sum;  // +50
    // result = 85
    
    // Test 4: Multiple evaluations with comma
    int count = 0;
    int val3 = (count = count + 1, count = count + 1, count = count + 1, count);
    result = result + val3;  // +3 (count is 3)
    // result = 88
    
    // Test 5: Comma in assignment
    int p, q;
    int r = (p = 5, q = 10, p + q);  // r = 15
    result = result + r;  // +15
    // result = 103
    
    // Test 6: Nested comma expressions
    int m = (x = 1, (y = 2, z = 3), x + y + z);  // m = 6
    result = result + m;  // +6
    // result = 109
    
    // Test 7: Comma with different types
    float f;
    int n = (f = 3.14, (int)f);  // n = 3
    result = result + n;  // +3
    // result = 112
    
    // Test 8: Comma in condition
    int cond_x = 5;
    int cond_y = 10;
    if ((cond_x = cond_x + 1, cond_y = cond_y - 1, cond_x < cond_y)) {
        result = result + 10;  // +10 (condition is true: 6 < 9)
    }
    // result = 122
    
    // Test 9: Return with comma
    int final = (1, 2, 3, 4, 5);  // final = 5
    result = result + final;  // +5
    // result = 127
    
    // Test 10: Comma with pointers
    int val_a = 42;
    int val_b = 100;
    int* ptr1;
    int* ptr2;
    int sum_ptr = (ptr1 = &val_a, ptr2 = &val_b, *ptr1 + *ptr2);  // sum_ptr = 142
    result = result + sum_ptr;  // +142
    // result = 269
    
    // Test 11: Order of evaluation test
    int order = 0;
    int eval = ((order = 1), (order = 2), (order = 3));  // eval = 3, order = 3
    result = result + eval + order;  // +6
    // result = 275
    
    // Test 12: Comma returns rightmost value
    int multi = (10, 20, 30, 40, 50);  // multi = 50
    result = result + multi;  // +50
    // result = 325
    
    printf("%d\n", result);
    return 0;
}
