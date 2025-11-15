// Test cast in division operations

int printf(char *format, ...);

int main() {
    int result = 0;

    // Division with cast
    int a = 5;
    int b = 2;
    int int_div = a / b;  // 2
    float float_div = (float)a / (float)b;  // 2.5
    int float_div_int = (int)float_div;  // 2
    
    result = result + int_div + float_div_int;  // +4
    // result = 4

    printf("%d\n", result);
    return 0;
}
