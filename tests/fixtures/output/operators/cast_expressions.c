// Test casts in complex expressions

int printf(char *format, ...);

int main() {
    int result = 0;

    // Cast in expressions
    int p = 10;
    int q = 3;
    double avg = ((double)p + (double)q) / 2.0;  // 6.5
    int avg_int = (int)avg;  // 6
    result = result + avg_int;  // +6
    // result = 6

    printf("%d\n", result);
    return 0;
}
