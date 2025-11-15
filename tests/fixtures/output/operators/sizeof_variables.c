// Test sizeof operator with variables
int printf(char *format, ...);
int test_sizeof_variable() {
    int x = 10;
    int size = sizeof(x);
    return size;
}

int test_sizeof_variable_different_types() {
    char c = 'a';
    double d = 3.14;
    int size1 = sizeof(c);
    int size2 = sizeof(d);
    return size1 + size2;
}

int main() {
    int totalSize = test_sizeof_variable() + test_sizeof_variable_different_types();
    printf("%d\n", totalSize);
    return 0;
}
