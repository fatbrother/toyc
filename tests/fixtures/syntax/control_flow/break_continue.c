// break 和 continue 語句測試

// 測試 break 在 for 循環中
int test_for_break() {
    int i;
    int sum = 0;
    
    for (i = 0; i < 10; i++) {
        if (i == 5) {
            break;  // 當 i 等於 5 時跳出循環
        }
        sum = sum + i;
    }
    
    return sum;  // 應該返回 0+1+2+3+4 = 10
}

// 測試 continue 在 for 循環中
int test_for_continue() {
    int i;
    int sum = 0;
    
    for (i = 0; i < 10; i++) {
        if (i == 5) {
            continue;  // 當 i 等於 5 時跳過本次迭代
        }
        sum = sum + i;
    }
    
    return sum;  // 應該返回 0+1+2+3+4+6+7+8+9 = 40
}

// 測試 break 在 while 循環中
int test_while_break() {
    int i = 0;
    int sum = 0;
    
    while (i < 10) {
        if (i == 5) {
            break;  // 當 i 等於 5 時跳出循環
        }
        sum = sum + i;
        i++;
    }
    
    return sum;  // 應該返回 0+1+2+3+4 = 10
}

// 測試 continue 在 while 循環中
int test_while_continue() {
    int i = 0;
    int sum = 0;
    
    while (i < 10) {
        i++;
        if (i == 5) {
            continue;  // 當 i 等於 5 時跳過本次迭代
        }
        sum = sum + i;
    }
    
    return sum;  // 應該返回 1+2+3+4+6+7+8+9+10 = 50
}

// 測試巢狀循環中的 break
int test_nested_break() {
    int i;
    int j;
    int count = 0;
    
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 5; j++) {
            if (j == 3) {
                break;  // 只跳出內層循環
            }
            count++;
        }
    }
    
    return count;  // 每次內層循環執行 3 次，共 5 次外層循環 = 15
}

// 測試巢狀循環中的 continue
int test_nested_continue() {
    int i;
    int j;
    int count = 0;
    
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 5; j++) {
            if (j == 2) {
                continue;  // 跳過 j=2 的迭代
            }
            count++;
        }
    }
    
    return count;  // 每次內層循環執行 4 次（跳過 j=2），共 5 次外層循環 = 20
}

// 測試複雜的條件中使用 break 和 continue
int test_complex_break_continue() {
    int i;
    int sum = 0;
    
    for (i = 0; i < 20; i++) {
        if (i < 5) {
            continue;  // 跳過前 5 個數字
        }
        
        if (i > 15) {
            break;  // 大於 15 時跳出
        }
        
        if (i == 10) {
            continue;  // 跳過 10
        }
        
        sum = sum + i;
    }
    
    return sum;  // 5+6+7+8+9+11+12+13+14+15 = 100
}

// 測試 do-while 循環中的 break
int test_do_while_break() {
    int i = 0;
    int sum = 0;
    
    do {
        if (i == 5) {
            break;
        }
        sum = sum + i;
        i++;
    } while (i < 10);
    
    return sum;  // 0+1+2+3+4 = 10
}

// 測試 do-while 循環中的 continue
int test_do_while_continue() {
    int i = 0;
    int sum = 0;
    
    do {
        i++;
        if (i == 5) {
            continue;
        }
        sum = sum + i;
    } while (i < 10);
    
    return sum;  // 1+2+3+4+6+7+8+9+10 = 50
}

int main() {
    int result;
    
    result = test_for_break();
    result = test_for_continue();
    result = test_while_break();
    result = test_while_continue();
    result = test_nested_break();
    result = test_nested_continue();
    result = test_complex_break_continue();
    result = test_do_while_break();
    result = test_do_while_continue();
    
    return 0;
}
