int printf(char *format, ...);

int main() {
    int a = 3;
    int b = 2;
    int arr[a + b];
    int i;
    
    for (i = 0; i < a + b; i = i + 1) {
        arr[i] = i + 1;
    }
    
    printf("%d\n", arr[0] + arr[4]);
    return 0;  // 應輸出 6 (1+5)
}
