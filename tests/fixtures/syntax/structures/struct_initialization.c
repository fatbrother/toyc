// struct 初始化測試
struct Point {
    int x;
    int y;
};

int main() {
    struct Point p1;
    struct Point p2;
    // 注意：這裡只測試語法解析，不涉及實際初始化語法
    p1.x = 10;
    p1.y = 20;
    return 0;
}
