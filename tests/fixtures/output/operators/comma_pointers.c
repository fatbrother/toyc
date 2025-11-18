// Test 10: Comma with pointers

int printf(char *format, ...);

int main() {
    int val_a = 42;
    int val_b = 100;
    int* ptr1;
    int* ptr2;
    int sum_ptr = (ptr1 = &val_a, ptr2 = &val_b, *ptr1 + *ptr2);  // sum_ptr = 142
    
    printf("%d\n", sum_ptr);  // Expected: 142
    return 0;
}
