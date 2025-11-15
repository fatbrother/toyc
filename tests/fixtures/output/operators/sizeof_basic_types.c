// Test sizeof operator with basic type names
int printf(char *format, ...);
int main() {
    int size_int = sizeof(int);        // 4
    int size_char = sizeof(char);      // 1
    int size_short = sizeof(short);    // 2
    int size_long = sizeof(long);      // 8
    int size_float = sizeof(float);    // 4
    int size_double = sizeof(double);  // 8
    int totalSize = size_int + size_char + size_short + size_long + size_float + size_double; // 27
    printf("%d\n", totalSize);
    return 0;
}
