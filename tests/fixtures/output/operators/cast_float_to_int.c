// Test float to integer type casts (truncation)

int printf(char *format, ...);

int main() {
    int result = 0;

    // Float to integer
    float f = 3.14;
    int fi = (int)f;  // 3
    result = result + fi;  // +3
    // result = 3

    // Double to integer
    double d = 9.99;
    int di = (int)d;  // 9
    result = result + di;  // +9
    // result = 12

    printf("%d\n", result);
    return 0;
}
