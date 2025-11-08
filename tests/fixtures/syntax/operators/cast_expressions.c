// Test type casting (cast expressions)
// Each function tests a specific cast syntax pattern

// Test 1: Basic type casts - integer to integer
int test_cast_int_to_char() {
    int i = 100;
    char c = (char)i;
    return c;
}

int test_cast_int_to_long() {
    int i = 42;
    long l = (long)i;
    return l;
}

// Test 2: Float/double casts
int test_cast_float_to_int() {
    float f = 3.14;
    int i = (int)f;
    return i;
}

int test_cast_int_to_float() {
    int x = 42;
    float f = (float)x;
    return x;
}

int test_cast_double_to_int() {
    double d = 9.99;
    int i = (int)d;
    return i;
}

// Test 3: Pointer casts
int test_cast_pointer_to_void() {
    int x = 42;
    int* ip = &x;
    void* vp = (void*)ip;
    return *ip;
}

int test_cast_void_to_pointer() {
    int x = 42;
    void* vp = (void*)&x;
    int* ip = (int*)vp;
    return *ip;
}

int test_cast_pointer_to_int() {
    int x = 42;
    long addr = (long)&x;
    return x;
}

// Test 4: Cast in expressions
int test_cast_in_division() {
    int a = 5;
    int b = 2;
    float result = (float)a / (float)b;
    return a;
}

int test_cast_in_arithmetic() {
    int x = (int)(3.14 * 2.5);
    return x;
}

// Test 5: Nested casts
int test_nested_cast() {
    double d = 3.14159;
    int i = (int)(float)d;
    return i;
}

// Test 6: Cast with struct pointers
int test_cast_struct_pointer() {
    struct Point {
        int x;
        int y;
    };
    struct Point p;
    p.x = 10;
    void* vp = (void*)&p;
    struct Point* pp = (struct Point*)vp;
    return pp->x;
}

// Test 7: Cast with array
int test_cast_array_pointer() {
    int arr[5];
    arr[0] = 100;
    int* p = (int*)arr;
    return *p;
}

// Test 8: Cast char to int (ASCII)
int test_cast_char_to_int() {
    char ch = 'A';
    int ascii = (int)ch;
    return ascii;
}

int test_cast_int_to_char_ascii() {
    int num = 65;
    char c = (char)num;
    return num;
}

// Test 9: Cast with unary operators
int test_cast_with_dereference() {
    int x = 10;
    int* px = &x;
    int value = (int)*px;
    return value;
}

int test_cast_with_negative() {
    int x = 10;
    float f = -(float)x;
    return x;
}

// Test 10: Cast in return
int test_cast_in_return() {
    float f = 3.14;
    return (int)f;
}

int main() {
    test_cast_int_to_char();
    test_cast_int_to_long();
    test_cast_float_to_int();
    test_cast_int_to_float();
    test_cast_double_to_int();
    test_cast_pointer_to_void();
    test_cast_void_to_pointer();
    test_cast_pointer_to_int();
    test_cast_in_division();
    test_cast_in_arithmetic();
    test_nested_cast();
    test_cast_struct_pointer();
    test_cast_array_pointer();
    test_cast_char_to_int();
    test_cast_int_to_char_ascii();
    test_cast_with_dereference();
    test_cast_with_negative();
    test_cast_in_return();
    return 0;
}
