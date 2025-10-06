// 變數作用域測試
int main() {
    int local_var = 10;  // 區域變數

    {
        int block_var = 20;  // 區塊作用域變數
        local_var = local_var + block_var;
    }

    if (local_var > 0) {
        int if_var = 30;     // if 區塊作用域
        local_var = local_var + if_var;
    }

    for (int i = 0; i < 5; i++) {  // for 迴圈作用域
        int loop_var = i * 2;
        local_var = local_var + loop_var;
    }

    return 0;
}