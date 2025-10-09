// struct 指標測試
struct Point {
    int x;
    int y;
};

int main() {
    struct Point p;
    struct Point *ptr;
    
    ptr = &p;
    ptr->x = 10;
    ptr->y = 20;
    
    return 0;
}
