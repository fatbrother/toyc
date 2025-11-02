// switch-case fall-through 測試
int main() {
    int x = 1;
    int result = 0;

    // 測試 fall-through 行為
    switch (x) {
        case 1:
            result = result + 10;
        case 2:
            result = result + 20;
            break;
        case 3:
            result = result + 30;
            break;
        default:
            result = 99;
            break;
    }

    return result;  // 應該返回 30 (10 + 20)
}
