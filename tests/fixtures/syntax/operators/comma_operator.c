// Test comma operator in expressions
// Each function tests a specific comma operator syntax pattern

// Test 1: Basic comma operator - evaluates and returns rightmost
int test_basic_comma() {
    int x, y, z;
    int result = (x = 1, y = 2, z = 3);
    return result;
}

// Test 2: Comma with two expressions
int test_comma_two_expr() {
    int a = 5;
    int result = (a = a + 1, a * 2);
    return result;
}

// Test 3: Comma in for loop initialization
int test_comma_for_init() {
    int i, j;
    for (i = 0, j = 10; i < 5; i++) {
        j = j - 1;
    }
    return i;
}

// Test 4: Comma in for loop increment
int test_comma_for_increment() {
    int i, j, sum = 0;
    for (i = 0, j = 10; i < 5; i++, j--) {
        sum = sum + 1;
    }
    return sum;
}

// Test 5: Comma with side effects
int test_comma_side_effects() {
    int a = 5;
    int b = 10;
    int result = (a = a + 1, b = b * 2, a + b);
    return result;
}

// Test 6: Comma in assignment right side
int test_comma_in_assignment() {
    int x = 0;
    int y = 0;
    int z = (x = 5, y = 10, x + y);
    return z;
}

// Test 7: Comma in if condition
int test_comma_in_condition() {
    int x = 5;
    int y = 10;
    if ((x = x + 1, y = y - 1, x < y)) {
        return 1;
    }
    return 0;
}

// Test 8: Comma in while condition
int test_comma_in_while() {
    int i = 0;
    int count = 3;
    while ((i = i + 1, count = count - 1, count > 0)) {
        // loop body
    }
    return i;
}

// Test 9: Comma in array subscript
int test_comma_in_subscript() {
    int arr[10];
    arr[5] = 500;
    int i = 0;
    int value = arr[(i = i + 1, i + 4)];
    return value;
}

// Test 10: Nested comma expressions
int test_nested_comma() {
    int a, b, c, d;
    int result = (a = 1, (b = 2, c = 3), d = 4, a + b + c + d);
    return result;
}

// Test 11: Comma with pointers
int test_comma_with_pointers() {
    int x = 42;
    int y = 100;
    int* p;
    int* q;
    int result = (p = &x, q = &y, *p + *q);
    return result;
}

// Test 12: Comma in return statement
int test_comma_in_return() {
    int x = 5;
    int y = 10;
    return (x = x + 1, y = y + 2, x + y);
}

// Test 13: Multiple comma operators
int test_multiple_comma() {
    int result = (1, 2, 3, 4, 5);
    return result;
}

// Test 14: Comma operator vs declaration comma
int test_comma_vs_declaration() {
    int x, y;  // Declaration comma
    int result = (x = 1, y = 2);  // Comma operator
    return result;
}

int main() {
    test_basic_comma();
    test_comma_two_expr();
    test_comma_for_init();
    test_comma_for_increment();
    test_comma_side_effects();
    test_comma_in_assignment();
    test_comma_in_condition();
    test_comma_in_while();
    test_comma_in_subscript();
    test_nested_comma();
    test_comma_with_pointers();
    test_comma_in_return();
    test_multiple_comma();
    test_comma_vs_declaration();
    return 0;
}
