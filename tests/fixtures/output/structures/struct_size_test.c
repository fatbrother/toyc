// struct 大小相關測試
struct Small {
    char c;
};

struct Medium {
    int i;
};

struct Large {
    int i;
    char c;
    double d;
};

int main() {
    struct Small s;
    struct Medium m;
    struct Large l;
    
    s.c = 'A';
    m.i = 42;
    l.i = 100;
    l.c = 'B';
    
    return m.i;  // 應該返回 42
}
