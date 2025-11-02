// 多個標籤的 goto
int main() {
    int x = 1;

    goto label2;

label1:
    x = x + 10;
    goto label3;

label2:
    x = x + 100;
    goto label1;

label3:
    return x;  // 預期輸出: 111 (1 + 100 + 10)
}
