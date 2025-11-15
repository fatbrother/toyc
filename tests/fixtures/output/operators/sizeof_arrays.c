// Test sizeof operator with arrays

int printf(char *format, ...);

int main() {
    int size1 = sizeof(int[10]);      // 10 * 4 = 40
    int size2 = sizeof(char[20]);     // 20 * 1 = 20
    int size3 = sizeof(double[5]);    // 5 * 8 = 40
    int totalSize = size1 + size2 + size3; // 100
    printf("%d\n", totalSize);
    return 0;
}
