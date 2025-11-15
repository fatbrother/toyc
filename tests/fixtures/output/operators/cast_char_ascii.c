// Test char to int casts (ASCII values)

int printf(char *format, ...);

int main() {
    int result = 0;

    // Char to int (ASCII values)
    char ch = 'A';  // ASCII 65
    int ascii = (int)ch;
    result = result + ascii;  // +65
    // result = 65

    // Int to char
    int num = 66;
    char ch2 = (char)num;  // 'B'
    result = result + (int)ch2;  // +66
    // result = 131

    printf("%d\n", result);
    return 0;
}
