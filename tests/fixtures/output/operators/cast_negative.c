// Test negative number casts

int printf(char *format, ...);

int main() {
    int result = 0;

    // Negative numbers
    int neg = -10;
    float negf = (float)neg;
    int negi = (int)negf;  // -10
    result = result + negi;  // -10
    // result = -10

    printf("%d\n", result);
    return 0;
}
