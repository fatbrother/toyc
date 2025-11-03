// 簡化版達夫裝置測試
// 測試 switch 和 do-while 的混合使用
int main() {
    int n = 5;
    int result = 0;
    int count = 0;

    // 根據 n 的值決定從哪個 case 開始執行
    switch (n) {
        case 5:
            result = result + 5;
            count = count + 1;
        case 4:
            result = result + 4;
            count = count + 1;
        case 3:
            result = result + 3;
            count = count + 1;
        case 2:
            result = result + 2;
            count = count + 1;
        case 1:
            result = result + 1;
            count = count + 1;
            break;
        default:
            result = 99;
            break;
    }

    // n=5: 執行 5+4+3+2+1 = 15, count = 5
    // 返回 result (15)
    return result;
}
