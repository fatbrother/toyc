// Test sizeof operator output

int printf(char *format, ...);

int main() {
    int result = 0;

    // Test 1: sizeof basic types (accumulate known sizes)
    // Assuming: char=1, short=2, int=4, long=8, float=4, double=8
    result = result + sizeof(char);      // +1
    result = result + sizeof(short);     // +2
    result = result + sizeof(int);       // +4
    result = result + sizeof(long);      // +8
    result = result + sizeof(float);     // +4
    result = result + sizeof(double);    // +8
    // result should be 27 (1+2+4+8+4+8)

    // Test 2: sizeof pointers (all pointers are 8 bytes on 64-bit)
    result = result + sizeof(char*);     // +8
    result = result + sizeof(int*);      // +8
    result = result + sizeof(double*);   // +8
    result = result + sizeof(void*);     // +8
    // result should be 59 (27+32)

    // Test 3: sizeof arrays
    int arr[10];
    result = result + sizeof(arr) / sizeof(int);  // +10 (40/4)
    // result should be 69 (59+10)

    // Test 4: sizeof variables
    int x = 42;
    result = result + sizeof(x);  // +4
    // result should be 73 (69+4)

    // Test 5: sizeof expressions
    int a = 5;
    int b = 10;
    result = result + sizeof(a + b);  // +4 (sizeof int)
    // result should be 77 (73+4)

    // Test 6: sizeof struct
    struct Point {
        int x;
        int y;
    };
    result = result + sizeof(struct Point);  // +8 (two ints)
    // result should be 85 (77+8)

    // Test 7: Verify calculation works
    int arr2[5];
    int num_elements = sizeof(arr2) / sizeof(int);  // 5
    result = result + num_elements;  // +5
    // result should be 90 (85+5)

    // Final result: 90
    printf("%d\n", result);
    return 0;
}
