// Test file for type qualifiers (const and volatile)
// This file demonstrates that const and volatile are properly parsed

int add_const_params(const int a, const int b) {
    return a + b;
}

int modify_volatile(volatile int *ptr) {
    *ptr = 42;
    return *ptr;
}

const int return_const_value() {
    return 100;
}

volatile int return_volatile_value() {
    return 200;
}

struct Point {
    const int x;
    volatile int y;
    int z;
};

int main() {
    // Basic const and volatile variables
    const int cx = 10;
    volatile int vx = 20;
    const volatile int cvx = 30;
    
    // Pointers with qualifiers
    const int *p1 = &cx;         // pointer to const int
    volatile int *p2 = &vx;      // pointer to volatile int
    int * const p3 = &vx;        // const pointer to int
    const int * const p4 = &cx;  // const pointer to const int
    
    // Function calls with const parameters
    int sum = add_const_params(5, 7);
    
    // Struct with qualified members
    struct Point pt;
    pt.x = 1;
    pt.y = 2;
    pt.z = 3;
    
    // Arrays with const
    const int arr[3];
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
    
    // Calculate result
    int result = cx + vx + cvx + *p1 + *p2 + *p3 + *p4 + 
                 sum + pt.x + pt.y + pt.z + arr[0] + arr[1] + arr[2];
    
    return result;
}
