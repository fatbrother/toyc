// Test: VLA declared inside loop
// Expected: Should handle VLA in loop scope

int printf(char *format, ...);
int main() {
    for (int i = 1; i <= 3; i++) {
        int arr[i * 2];
        int size = i * 2;

        for (int j = 0; j < size; j++) {
            arr[j] = j;
        }

        printf("Iteration %d, size %d: ", i, size);
        for (int j = 0; j < size; j++) {
            printf("%d ", arr[j]);
        }
        printf("\n");
    }

    return 0;
}
