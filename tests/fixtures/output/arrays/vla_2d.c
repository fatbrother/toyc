// Test: VLA simulating a 2D matrix with flat indexing
// Expected: Demonstrates multi-row VLA using flat array + manual index computation

int printf(char *format, ...);
int main() {
    int rows = 3;
    int cols = 4;
    int matrix[rows * cols];

    for (int i = 0; i < rows; i = i + 1) {
        for (int j = 0; j < cols; j = j + 1) {
            matrix[i * cols + j] = i * cols + j;
        }
    }

    for (int i = 0; i < rows; i = i + 1) {
        for (int j = 0; j < cols; j = j + 1) {
            printf("%d ", matrix[i * cols + j]);
        }
        printf("\n");
    }

    return 0;
}
