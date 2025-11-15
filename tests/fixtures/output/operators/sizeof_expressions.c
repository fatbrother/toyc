// Test sizeof operator with expressions (not evaluated)
int printf(char *format, ...);
int test_sizeof_expression() {
    int x = 5;
    int y = 10;
    int size = sizeof(x + y);
    return size;
}

int test_sizeof_complex_expression() {
    int a = 1;
    int b = 2;
    int c = 3;
    int size = sizeof(a * b + c);
    return size;
}

int main() {
    int totalSize = test_sizeof_expression() + test_sizeof_complex_expression();
    printf("%d\n", totalSize);
    return 0;
}
