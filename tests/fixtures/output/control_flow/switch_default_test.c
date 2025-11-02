// switch-case default 測試
int main() {
    int x = 10;
    int result = 0;

    // 測試 default 分支
    switch (x) {
        case 1:
            result = 1;
            break;
        case 2:
            result = 2;
            break;
        case 3:
            result = 3;
            break;
        default:
            result = 99;
            break;
    }

    return result;  // 應該返回 99
}
