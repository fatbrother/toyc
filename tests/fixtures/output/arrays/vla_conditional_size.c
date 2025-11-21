int printf(char *format, ...);

int main() {
    int condition = 1;
    int size = condition ? 5 : 3;
    int arr[size];
    int i;
    
    for (i = 0; i < size; i = i + 1) {
        arr[i] = i * i;
    }
    
    printf("%d\n", arr[2] + arr[4]);
    return 0;  // 應輸出 20 (4+16)
}
