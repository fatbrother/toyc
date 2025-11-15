// Test integer to integer type casts

int printf(char *format, ...);

int main() {
    int result = 0;

    // Integer to different integer types
    int i = 300;
    char c = (char)i;      // Will truncate to 44 (300 % 256)
    short s = (short)i;    // 300
    long l = (long)i;      // 300

    result = result + (int)c;   // +44
    result = result + (int)s;   // +300
    result = result + (int)l;   // +300
    // result = 644

printf("%d\n", result);
    return 0;
}
