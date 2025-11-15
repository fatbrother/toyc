// Test sizeof operator used in arithmetic and comparison expressions
int printf(char *format, ...);
int test_sizeof_in_arithmetic() {
    int arr[10];
    int count = sizeof(arr) / sizeof(int);
    return count;
}

int test_sizeof_in_comparison() {
    if (sizeof(int) == 4) {
        return 1;
    }
    return 0;
}

int test_sizeof_multiple_operations() {
    int x = sizeof(int) + sizeof(char);
    int y = sizeof(double) - sizeof(int);
    return x + y;
}

int main() {
    int total = test_sizeof_in_arithmetic() +
                test_sizeof_in_comparison() +
                test_sizeof_multiple_operations();
    printf("%d\n", total);
    return 0;
}
