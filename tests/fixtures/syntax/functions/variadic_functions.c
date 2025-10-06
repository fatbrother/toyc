// 可變參數函數測試（使用省略號）
int sum_all(int first, ...);

int main() {
    // 注意：實際的可變參數處理需要額外的實作
    // 這裡只測試語法解析
    return 0;
}

int sum_all(int first, ...) {
    return first;
}