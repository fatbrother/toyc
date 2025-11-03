// 達夫裝置 (Duff's Device) 測試
// 這是一個經典的優化技巧，結合了 switch 和循環
int main() {
    int count = 8;
    int result = 0;
    int n = count;

    // 簡化版達夫裝置
    // 每次迭代累加當前的 n 值
    int iterations = (n + 7) / 8;  // 向上取整到 8 的倍數

    switch (n % 8) {
        case 0:
            do {
                result = result + n;
                n = n - 1;
        case 7:
                result = result + n;
                n = n - 1;
        case 6:
                result = result + n;
                n = n - 1;
        case 5:
                result = result + n;
                n = n - 1;
        case 4:
                result = result + n;
                n = n - 1;
        case 3:
                result = result + n;
                n = n - 1;
        case 2:
                result = result + n;
                n = n - 1;
        case 1:
                result = result + n;
                n = n - 1;
            } while (n > 0);
    }

    // count = 8: 應該累加 8+7+6+5+4+3+2+1 = 36
    return result;
}
