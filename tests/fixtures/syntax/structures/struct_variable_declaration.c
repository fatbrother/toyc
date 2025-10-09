// struct 變數聲明的各種方式測試
struct Point {
    int x;
    int y;
} p1, p2;

struct {
    int width;
    int height;
} rect1, rect2;

int main() {
    struct Point p3, p4;
    struct Point *ptr1, *ptr2;
    
    return 0;
}
