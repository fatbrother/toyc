// while 迴圈測試
int main() {
    int count = 0;
    int sum = 0;
    
    // 基本 while 迴圈
    while (count < 10) {
        sum = sum + count;
        count++;
    }
    
    // 巢狀 while 迴圈
    int i = 0;
    while (i < 3) {
        int j = 0;
        while (j < 3) {
            sum = sum + i + j;
            j++;
        }
        i++;
    }
    
    return 0;
}