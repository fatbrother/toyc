int printf(char *format, ...);

int main() {
    int val = 7;
    int * const p = &val;
    printf("%d\n", *p);
    return 0;
}
