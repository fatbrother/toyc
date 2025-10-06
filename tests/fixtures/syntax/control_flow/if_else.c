// if-else 語句測試
int main() {
    int x = 10;
    
    // 基本 if 語句
    if (x > 5) {
        x = x + 1;
    }
    
    // if-else 語句
    if (x > 15) {
        x = x - 5;
    } else {
        x = x + 5;
    }
    
    // 巢狀 if-else
    if (x > 10) {
        if (x > 20) {
            x = 20;
        } else {
            x = 15;
        }
    } else {
        x = 5;
    }
    
    return 0;
}