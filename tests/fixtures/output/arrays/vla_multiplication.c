int printf(char *format, ...);

int main() {
    int x = 2;
    int y = 3;
    int arr[x * y];
    int i;
    
    for (i = 0; i < x * y; i = i + 1) {
        arr[i] = i + 10;
    }
    
    printf("%d\n", arr[0] + arr[5]);
    return 0;  // 應輸出 25 (10+15)
}
