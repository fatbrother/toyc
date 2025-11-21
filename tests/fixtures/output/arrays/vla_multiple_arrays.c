int printf(char *format, ...);

int main() {
    int n = 4;
    int arr1[n];
    int arr2[n + 1];
    int i;
    
    for (i = 0; i < n; i = i + 1) {
        arr1[i] = i;
    }
    
    for (i = 0; i < n + 1; i = i + 1) {
        arr2[i] = i * 2;
    }
    
    printf("%d\n", arr1[3] + arr2[4]);
    return 0;  // 應輸出 11 (3+8)
}
