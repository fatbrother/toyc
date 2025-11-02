// switch-case 語句測試
int main() {
    int x = 2;
    int result = 0;

    // 基本 switch 語句
    switch (x) {
        case 1:
            result = 10;
            break;
        case 2:
            result = 20;
            break;
        case 3:
            result = 30;
            break;
        default:
            result = 0;
            break;
    }

    // switch with fall-through
    int y = 1;
    int flag = 0;
    switch (y) {
        case 1:
            flag = flag + 1;
        case 2:
            flag = flag + 2;
            break;
        case 3:
            flag = flag + 3;
            break;
    }

    // 巢狀 switch
    int a = 1;
    int b = 2;
    int nested_result = 0;
    switch (a) {
        case 1:
            switch (b) {
                case 1:
                    nested_result = 11;
                    break;
                case 2:
                    nested_result = 12;
                    break;
            }
            break;
        case 2:
            nested_result = 20;
            break;
    }

    // switch without default
    int z = 5;
    int no_default = 0;
    switch (z) {
        case 1:
            no_default = 1;
            break;
        case 2:
            no_default = 2;
            break;
    }

    // switch with multiple statements per case
    int m = 1;
    int multi = 0;
    switch (m) {
        case 1:
            multi = multi + 1;
            multi = multi * 2;
            multi = multi + 3;
            break;
        case 2:
            multi = multi + 10;
            break;
    }

    return 0;
}
