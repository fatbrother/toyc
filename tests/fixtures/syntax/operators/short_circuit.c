// 短路求值測試 - 完整測試案例

// 測試 AND 短路（第一個為假）
int test_and_short_circuit_false() {
    int count = 0;
    int result = 0 && (count = count + 1);  // 右側不應被求值
    return count;  // 應該返回 0
}

// 測試 AND 不短路（第一個為真）
int test_and_no_short_circuit() {
    int count = 0;
    int result = 1 && (count = count + 1);  // 右側應該被求值
    return count;  // 應該返回 1
}

// 測試 OR 短路（第一個為真）
int test_or_short_circuit_true() {
    int count = 0;
    int result = 1 || (count = count + 1);  // 右側不應被求值
    return count;  // 應該返回 0
}

// 測試 OR 不短路（第一個為假）
int test_or_no_short_circuit() {
    int count = 0;
    int result = 0 || (count = count + 1);  // 右側應該被求值
    return count;  // 應該返回 1
}

// 測試複雜的短路求值
int test_complex_short_circuit() {
    int count_a = 0;
    int count_b = 0;
    
    // (0 && (count_a++)) || (count_b++)
    // 0 && ... 短路，count_a 不變
    // 0 || (count_b++) 不短路，count_b 增加
    int result = (0 && (count_a = count_a + 1)) || (count_b = count_b + 1);
    
    // count_a 應該是 0（未求值）
    // count_b 應該是 1（被求值）
    return count_a * 10 + count_b;  // 應該返回 1
}

// 測試巢狀短路求值
int test_nested_short_circuit() {
    int count_a = 0;
    int count_b = 0;
    
    // 1 && (0 || (count_a++)) && (count_b++)
    // 1 && ... 繼續評估
    // 0 || (count_a++) 求值 count_a++，返回 1
    // 1 && (count_b++) 求值 count_b++，返回 1
    int result = 1 && (0 || (count_a = count_a + 1)) && (count_b = count_b + 1);
    
    // 兩個計數器都應該增加
    return count_a + count_b;  // 應該返回 2
}

// 測試短路與條件語句結合
int test_short_circuit_in_condition() {
    int x = 5;
    int y = 0;
    
    // 防止除以零：y != 0 為假時短路，不會執行 x/y
    if (y != 0 && x / y > 2) {
        return 1;
    }
    
    return 0;  // 應該返回 0
}

int main() {
    int result;
    
    result = test_and_short_circuit_false();      // 0
    result = test_and_no_short_circuit();         // 1
    result = test_or_short_circuit_true();        // 0
    result = test_or_no_short_circuit();          // 1
    result = test_complex_short_circuit();        // 1
    result = test_nested_short_circuit();         // 2
    result = test_short_circuit_in_condition();   // 0
    
    return 0;
}
