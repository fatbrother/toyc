// Test multiple consecutive casts

int printf(char *format, ...);

int main() {
    int result = 0;

    // Multiple casts
    double dd = 3.14159;
    int ii = (int)(float)dd;  // 3
    result = result + ii;  // +3
    // result = 3

    // Zero cast
    int zero = 0;
    float zerof = (float)zero;
    int zeroi = (int)zerof;  // 0
    result = result + zeroi;  // +0
    // result = 3

    printf("%d\n", result);
    return 0;
}
