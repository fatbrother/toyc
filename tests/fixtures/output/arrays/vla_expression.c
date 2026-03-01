// Test: VLA with expression as size
// Expected: Should accept arithmetic expressions for array size

int printf(char *format, ...);

int main() {
    int x = 3;
    int y = 2;
    int arr[x + y];  // Size is 5

    int size = x + y;

    for (int i = 0; i < size; i++) {
        arr[i] = i + 1;
    }

    for (int i = 0; i < size; i++) {
        printf("%d\n", arr[i]);
    }

    return 0;
}
