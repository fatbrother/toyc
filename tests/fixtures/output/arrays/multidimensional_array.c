int main() {
    int matrix[2][3];

    matrix[0][0] = 1;
    matrix[0][1] = 2;
    matrix[0][2] = 3;
    matrix[1][0] = 4;
    matrix[1][1] = 5;
    matrix[1][2] = 6;

    return matrix[0][0] + matrix[1][2];  // 應返回 7 (1+6)
}
