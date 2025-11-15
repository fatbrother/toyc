// Test: Basic Variable Length Array (VLA)
// Expected: Should declare and use a VLA with runtime size

int printf(char *format, ...);
int main() {
    int n = 5;
    int arr[n];

    // Initialize array
    for (int i = 0; i < n; i++) {
        arr[i] = i * 10;
    }

    // Print array elements
    for (int i = 0; i < n; i++) {
        printf("%d\n", arr[i]);
    }

    return 0;
}
