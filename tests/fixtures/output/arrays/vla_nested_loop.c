int printf(char *format, ...);

int main() {
    int rows = 3;
    int cols = 4;
    int arr[rows * cols];
    int i;
    int j;
    int sum = 0;
    
    for (i = 0; i < rows; i = i + 1) {
        for (j = 0; j < cols; j = j + 1) {
            arr[i * cols + j] = i + j;
        }
    }
    
    for (i = 0; i < rows * cols; i = i + 1) {
        sum = sum + arr[i];
    }
    
    printf("%d\n", sum);
    return 0;  // 應輸出 30
}
