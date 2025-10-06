// do-while 迴圈測試
int main() {
    int count = 0;
    int sum = 0;
    
    // 基本 do-while 迴圈
    do {
        sum = sum + count;
        count++;
    } while (count < 5);
    
    // 至少執行一次的情況
    int x = 10;
    do {
        x = x - 1;
    } while (x > 15);  // 條件為假，但仍執行一次
    
    return 0;
}