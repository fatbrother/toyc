// struct 作為返回值測試
struct Point {
    int x;
    int y;
};

int printf(char *format, ...);

struct Point createPoint(int x, int y) {
    struct Point p;
    p.x = x;
    p.y = y;
    return p;
}

int getPointSum(struct Point p) {
    return p.x + p.y;
}

int main() {
    struct Point p = createPoint(15, 25);
    printf("%d\n", getPointSum(p));
    return 0;  // 應該返回 40
}
