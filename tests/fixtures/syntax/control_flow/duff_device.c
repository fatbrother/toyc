// 達夫裝置語法測試
// 測試 switch 和 do-while 的混合使用（不含循環變數）
int test_duff_device() {
    int n = 3;
    int result = 0;

    // 達夫裝置模式：switch 和 do-while 混合
    switch (n) {
        case 5:
            result = result + 5;
        case 4:
            result = result + 4;
        case 3:
            do {
                result = result + 3;
                n = n - 1;
        case 2:
                result = result + 2;
                n = n - 1;
        case 1:
                result = result + 1;
                n = n - 1;
            } while (n > 0);
            break;
        default:
            result = 99;
            break;
    }

    return result;
}

// 簡化版達夫裝置
int test_simple_duff() {
    int n = 4;
    int result = 0;

    switch (n % 4) {
        case 0:
            result = result + 1;
        case 3:
            result = result + 2;
        case 2:
            result = result + 3;
        case 1:
            result = result + 4;
            break;
    }

    return result;
}

int main() {
    int r1 = test_duff_device();
    int r2 = test_simple_duff();
    return 0;
}
