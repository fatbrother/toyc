// while 迴圈測試
int main() {
    int count = 0;
    int i = 1;
    while (i <= 4) {
        count = count + i;
        i++;
    }
    return count;  // 應該返回 10 (1+2+3+4)
}