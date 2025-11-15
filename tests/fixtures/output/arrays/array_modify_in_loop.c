int printf(char *format, ...);

int main() {
    int arr[3] = {10, 20, 30};
    int i;

    // 每個元素乘以 2
    for (i = 0; i < 3; i = i + 1) {
        arr[i] = arr[i] * 2;
    }

    printf("%d\n", arr[0] + arr[1] + arr[2]);
    return 0;  // 應返回 120 (20+40+60)
}
