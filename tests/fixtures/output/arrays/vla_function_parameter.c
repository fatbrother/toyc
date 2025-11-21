int printf(char *format, ...);

int sum_vla(int n, int arr[]) {
    int sum = 0;
    int i;
    for (i = 0; i < n; i = i + 1) {
        sum = sum + arr[i];
    }
    return sum;
}

int main() {
    int size = 5;
    int numbers[size];
    int i;

    for (i = 0; i < size; i = i + 1) {
        numbers[i] = i * 3;
    }

    printf("%d\n", sum_vla(size, numbers));
    return 0;  // 應輸出 30 (0+3+6+9+12)
}
