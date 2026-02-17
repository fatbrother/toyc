int main() {
    int x = 10;
    int * const p = &x;
    p = 0;
    return 0;
}
