int printf(char *format, ...);

int main() {
    int arr[5] = {1, 2, 3, 4, 5};
printf("%d\n", arr[0] + arr[4]);
    return 0;  // 應返回 6 (1+5)
}
