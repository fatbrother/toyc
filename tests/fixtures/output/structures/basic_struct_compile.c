// 基本 struct 編譯測試
struct Point {
    int x;
    int y;
};

int printf(char *format, ...);

int main() {
    struct Point p;
    p.x = 10;
    p.y = 20;
    printf("%d\n", p.x + p.y);
    return 0;  // 應該返回 30
}
