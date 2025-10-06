// 完整的 C 程式範例
int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

int is_prime(int n) {
    if (n <= 1) {
        return 0;
    }

    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) {
            return 0;
        }
    }

    return 1;
}

int main() {
    int size = 6;

    int total = 0;
    for (int i = 0; i < size; i++) {

        if (is_prime(i)) {
            total += factorial(i);
        } else {
            total += fibonacci(i);
        }
    }

    int *result_ptr = &total;
    int final_result = (*result_ptr > 1000) ? *result_ptr / 2 : *result_ptr * 2;

    return final_result;
}