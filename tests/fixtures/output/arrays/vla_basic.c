int printf(char *format, ...);

int main() {
    int n = 5;
    int arr[n];

    for (int i = 0; i < n; i = i + 1) {
        arr[i] = i * 2;
    }

    printf("%d\n", arr[0] + arr[4]);
    return 0;  // 應輸出 8 (0+8)
}
