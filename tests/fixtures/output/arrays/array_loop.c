int main() {
    int arr[5];
    int i;
    int sum = 0;

    for (i = 0; i < 5; i = i + 1) {
        arr[i] = i * 2;
    }

    for (i = 0; i < 5; i = i + 1) {
        sum = sum + arr[i];
    }

    return sum;  // 應返回 20 (0+2+4+6+8)
}
