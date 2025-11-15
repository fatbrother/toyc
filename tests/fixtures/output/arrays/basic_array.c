int printf(char *format, ...);

int main() {
    int arr[5];
    arr[0] = 10;
    arr[1] = 20;
    arr[2] = 30;
    printf("%d\n", arr[0] + arr[1] + arr[2]);
    return 0;  // 應返回 60
}
