// for 迴圈測試
int main() {
    int sum = 0;
    int i;
    
    // 基本 for 迴圈
    for (i = 0; i < 10; i++) {
        sum = sum + i;
    }
    
    // 在迴圈內聲明變數
    for (int j = 0; j < 5; j++) {
        sum = sum + j;
    }
    
    // 巢狀 for 迴圈
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            sum = sum + x * y;
        }
    }
    
    return 0;
}