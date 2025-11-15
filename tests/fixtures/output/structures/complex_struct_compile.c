// 複雜 struct 編譯和執行測試
struct Point {
    int x;
    int y;
};

struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
};

int printf(char *format, ...);

int calculateArea(struct Rectangle rect) {
    int width = rect.bottom_right.x - rect.top_left.x;
    int height = rect.bottom_right.y - rect.top_left.y;
    return width * height;
}

int main() {
    struct Rectangle rect;
    
    rect.top_left.x = 0;
    rect.top_left.y = 0;
    rect.bottom_right.x = 10;
    rect.bottom_right.y = 5;
    
    printf("%d\n", calculateArea(rect));
    return 0;  // 應該返回 50
}
