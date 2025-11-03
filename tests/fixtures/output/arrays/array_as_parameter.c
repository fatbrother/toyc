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
    return sum_array(numbers, 4);  // 應返回 50
}
