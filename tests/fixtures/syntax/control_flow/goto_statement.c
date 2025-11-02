// goto 語句測試

// 簡單的 goto 跳轉
int test_simple_goto() {
    int x = 0;
    goto skip;
    x = 10;  // 不會執行
skip:
    x = x + 5;
    return x;  // 應該返回 5
}

// 向前跳轉
int test_forward_goto() {
    int result = 0;
    goto forward;
    result = 100;  // 不會執行
forward:
    result = 42;
    return result;  // 返回 42
}

// 向後跳轉（模擬循環）
int test_backward_goto() {
    int sum = 0;
    int i = 0;
loop_start:
    if (i >= 5) {
        goto loop_end;
    }
    sum = sum + i;
    i = i + 1;
    goto loop_start;
loop_end:
    return sum;  // 應該返回 0+1+2+3+4 = 10
}

// 錯誤處理模式
int test_goto_error_handling() {
    int result = 0;
    int error = 0;

    if (error) {
        goto cleanup;
    }

    result = 42;

cleanup:
    return result;  // 返回 42
}

// 多個標籤
int test_multiple_labels() {
    int x = 1;

    goto label2;

label1:
    x = x + 10;
    goto label3;

label2:
    x = x + 100;
    goto label1;

label3:
    return x;  // 應該返回 111 (1 + 100 + 10)
}

// 跳出巢狀結構
int test_goto_out_of_nested() {
    int i;
    int j;
    int found = 0;

    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            if (i == 3 && j == 5) {
                found = i + j;
                goto found_it;
            }
        }
    }

found_it:
    return found;  // 應該返回 8 (3 + 5)
}

// if-else 中的 goto
int test_goto_in_if_else() {
    int x = 5;

    if (x > 10) {
        goto large;
    } else {
        goto small;
    }

large:
    return 100;

small:
    return 1;  // 應該返回 1
}

// 跳過變數聲明（注意：這在 C 中是合法的但可能有問題）
int test_goto_skip_declaration() {
    int x = 1;
    goto skip_decl;

    int y = 10;  // 會被跳過

skip_decl:
    x = x + 2;
    return x;  // 應該返回 3
}

// goto 和 break/continue 混合
int test_goto_with_loop_controls() {
    int sum = 0;
    int i;

    for (i = 0; i < 20; i++) {
        if (i < 5) {
            continue;  // 跳過前 5 個
        }

        if (i > 15) {
            goto done;  // 大於 15 時跳出
        }

        if (i == 10) {
            continue;  // 跳過 10
        }

        sum = sum + i;
    }

done:
    return sum;  // 5+6+7+8+9+11+12+13+14+15 = 100
}

int main() {
    int result;

    result = test_simple_goto();
    result = test_forward_goto();
    result = test_backward_goto();
    result = test_goto_error_handling();
    result = test_multiple_labels();
    result = test_goto_out_of_nested();
    result = test_goto_in_if_else();
    result = test_goto_skip_declaration();
    result = test_goto_with_loop_controls();

    return 0;
}
