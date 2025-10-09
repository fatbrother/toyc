// 嵌套 struct 測試
struct Point {
    int x;
    int y;
};

struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
};

int main() {
    struct Rectangle rect;
    rect.top_left.x = 0;
    rect.top_left.y = 0;
    rect.bottom_right.x = 100;
    rect.bottom_right.y = 50;
    return 0;
}
