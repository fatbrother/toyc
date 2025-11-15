// Test sizeof operator with struct types
int printf(char *format, ...);
int test_sizeof_struct() {
    struct Point {
        int x;
        int y;
    };
    int size = sizeof(struct Point);
    return size;
}

int test_sizeof_struct_var() {
    struct Point {
        int x;
        int y;
    };
    struct Point p;
    int size = sizeof(p);
    return size;
}

int main() {
    int totalSize = test_sizeof_struct() + test_sizeof_struct_var();
    printf("%d\n", totalSize);
    return 0;
}
