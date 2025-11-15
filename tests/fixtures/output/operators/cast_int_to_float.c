// Test integer to float type casts

int printf(char *format, ...);

int main() {
    int result = 0;

    // Integer to float and back
    int x = 42;
    float xf = (float)x;
    int xi = (int)xf;
    result = result + xi;  // +42
    // result = 42

    printf("%d\n", result);
    return 0;
}
