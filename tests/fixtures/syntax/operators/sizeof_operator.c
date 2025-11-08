// Test sizeof operator with different types and expressions
// Each function tests a specific sizeof syntax pattern

// Test 1: sizeof with basic type names in parentheses
int test_sizeof_type_char() {
    int size = sizeof(char);
    return size;
}

int test_sizeof_type_int() {
    int size = sizeof(int);
    return size;
}

int test_sizeof_type_double() {
    int size = sizeof(double);
    return size;
}

// Test 2: sizeof with pointer types
int test_sizeof_pointer() {
    int size = sizeof(int*);
    return size;
}

int test_sizeof_pointer_to_pointer() {
    int size = sizeof(int**);
    return size;
}

// Test 3: sizeof with variables
int test_sizeof_variable() {
    int x = 10;
    int size = sizeof(x);
    return size;
}

// Test 4: sizeof with expressions (not evaluated)
int test_sizeof_expression() {
    int x = 5;
    int y = 10;
    int size = sizeof(x + y);
    return size;
}

// Test 5: sizeof with array variable
int test_sizeof_array() {
    int arr[10];
    int size = sizeof(arr);
    return size;
}

// Test 6: sizeof with array type
int test_sizeof_array_type() {
    int size = sizeof(int[5]);
    return size;
}

// Test 7: sizeof with struct type
int test_sizeof_struct() {
    struct Point {
        int x;
        int y;
    };
    int size = sizeof(struct Point);
    return size;
}

// Test 8: sizeof with struct variable
int test_sizeof_struct_var() {
    struct Point {
        int x;
        int y;
    };
    struct Point p;
    int size = sizeof(p);
    return size;
}

// Test 9: sizeof in arithmetic expression
int test_sizeof_in_arithmetic() {
    int arr[10];
    int count = sizeof(arr) / sizeof(int);
    return count;
}

// Test 10: sizeof in comparison
int test_sizeof_in_comparison() {
    if (sizeof(int) == 4) {
        return 1;
    }
    return 0;
}

int main() {
    test_sizeof_type_char();
    test_sizeof_type_int();
    test_sizeof_type_double();
    test_sizeof_pointer();
    test_sizeof_pointer_to_pointer();
    test_sizeof_variable();
    test_sizeof_expression();
    test_sizeof_array();
    test_sizeof_array_type();
    test_sizeof_struct();
    test_sizeof_struct_var();
    test_sizeof_in_arithmetic();
    test_sizeof_in_comparison();
    return 0;
}

