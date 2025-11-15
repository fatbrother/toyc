// Test sizeof operator with arrays

int test_sizeof_array() {
    int arr[10];
    int size = sizeof(arr);
    return size;
}

int test_sizeof_array_type() {
    int size = sizeof(int[5]);
    return size;
}

int test_array_element_count() {
    int arr[10];
    int count = sizeof(arr) / sizeof(int);
    return count;
}

int main() {
    test_sizeof_array();
    test_sizeof_array_type();
    test_array_element_count();
    return 0;
}
