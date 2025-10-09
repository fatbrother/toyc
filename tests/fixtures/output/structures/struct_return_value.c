// struct 作為返回值測試
struct Point {
    int x;
    int y;
};

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
    return getPointSum(p);  // 應該返回 40
}
