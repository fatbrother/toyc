// struct 作為函數參數測試
struct Point {
    int x;
    int y;
};

void printPoint(struct Point p) {
    // 函數體省略
}

void modifyPoint(struct Point *p) {
    p->x = 100;
    p->y = 200;
}

int main() {
    struct Point p;
    p.x = 10;
    p.y = 20;
    
    printPoint(p);
    modifyPoint(&p);
    
    return 0;
}
