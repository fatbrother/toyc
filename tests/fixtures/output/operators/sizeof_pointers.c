// Test sizeof operator with pointer types
int printf(char *format, ...);
int test_sizeof_pointer() {
    int size = sizeof(int*);
    return size;
}

int test_sizeof_pointer_to_pointer() {
    int size = sizeof(int**);
    return size;
}

int main() {
    int totalSize = test_sizeof_pointer() + test_sizeof_pointer_to_pointer();
    printf("%d\n", totalSize);
    return 0;
}
