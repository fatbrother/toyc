// Test: 2D Variable Length Array
// Expected: Should support multidimensional VLA

int printf(char *format, ...);
int main() {
    int rows = 3;
    int cols = 4;
    int matrix[rows][cols];

    // Initialize 2D array
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = i * cols + j;
        }
    }

    // Print 2D array
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }

    return 0;
}
