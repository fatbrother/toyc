int printf(char *format, ...);

int sum_array(int arr[], int size) {
    int sum = 0;
    int i;
    for (i = 0; i < size; i = i + 1) {
        sum = sum + arr[i];
    }
    return sum;
}

int main() {
    int numbers[4] = {5, 10, 15, 20};
printf("%d\n", sum_array(numbers, 4));
    return 0;  // 應返回 50
}
