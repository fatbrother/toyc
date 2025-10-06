# ToyC 測試框架

這個目錄包含 ToyC 編譯器的測試套件，使用 Google Test 框架和 fixtures 資料夾結構來提供清晰的測試組織。

## 執行測試

```bash
# 編譯並執行所有測試
make test

# 或者分別編譯和執行
make build_test
./build/test_main
```

## 測試結構

### 測試檔案組織
- `main_test.cpp` - 主要測試入口點，整合所有測試模組
- `test_preprocessor.cpp` - 預處理器測試
- `test_error_handler.cpp` - 錯誤處理器測試
- `test_syntax.cpp` - C 語法解析測試
- `test_output.cpp` - 編譯器輸出測試
- `test_compiler_errors.cpp` - 編譯錯誤處理測試

### Fixtures 資料夾結構
測試使用 fixtures 資料夾來存放測試檔案，提供更好的可讀性和維護性：

```
tests/fixtures/
├── preprocessor/          # 預處理器測試檔案
│   ├── basic_macro.c      # 基本巨集測試
│   ├── include_test.c     # include 指令測試
│   ├── conditional_*.c    # 條件編譯測試
│   └── ...
├── error_handler/         # 錯誤處理器測試檔案
│   ├── basic_syntax_error.c      # 基本語法錯誤
│   ├── tab_syntax_error.c        # Tab 字符錯誤
│   ├── mixed_indentation_error.c # 混合縮排錯誤
│   └── ...
└── syntax/                # C 語法測試檔案
    ├── data_types/        # 資料類型測試
    ├── operators/         # 運算符測試
    ├── control_flow/      # 控制流測試
    ├── functions/         # 函數測試
    ├── variables/         # 變數測試
    ├── literals/          # 字面值測試
    └── *.c               # 複合測試檔案
└── output/                # 編譯器輸出測試檔案
    ├── simple_programs/   # 簡單程式測試
    ├── calculations/      # 計算結果測試
    ├── functions/         # 函數功能測試
    ├── control_flow/      # 控制流功能測試
    └── error_cases/       # 錯誤情況測試
```

## 預處理器測試

預處理器測試驗證 C 預處理器的各種功能：

### 基本功能
- **巨集定義與展開**: 測試 `#define` 指令的基本功能
- **檔案包含**: 測試 `#include` 指令
- **條件編譯**: 測試 `#if`, `#ifdef`, `#ifndef`, `#else`, `#elif`, `#endif`
- **註解處理**: 驗證單行和多行註解的正確移除

### 高級功能
- **函數式巨集**: 支援參數的巨集定義
- **預定義巨集**: `__FILE__`, `__LINE__` 等
- **defined 運算子**: 在條件編譯中檢查巨集是否定義
- **行延續**: 使用 `\` 進行多行巨集定義

### 測試檔案範例

```c
// tests/fixtures/preprocessor/basic_macro.c
#define MAX 100
#define MIN 0

int main() {
    int value = MAX;  // 應該展開為 100
    return 0;
}
```

## 錯誤處理器測試

錯誤處理器測試驗證編譯器能夠準確地標示錯誤位置：

### 測試項目
1. **基本錯誤位置**: 驗證錯誤指示器的準確性
2. **Tab 字符處理**: 正確處理 Tab 字符的視覺對齊
3. **混合縮排**: 處理 Tab 和空格混合的情況
4. **多行錯誤**: 跨行錯誤的正確標示
5. **中文字符**: 包含寬字符的錯誤處理

### 錯誤處理器行為
ErrorHandler 應該能夠：
- 準確顯示錯誤發生的檔案名稱和位置
- 顯示包含錯誤的原始程式碼行
- 使用 `^` 符號準確指向錯誤位置
- 正確處理 Tab 字符，直接輸出到終端以保持對齊
- 處理混合縮排的情況

### 測試檔案範例

```c
// tests/fixtures/error_handler/tab_syntax_error.c
int main() {
	int x =  // 錯誤：缺少表達式
	return 0;
}
```

預期輸出：
```
Error at tests/fixtures/error_handler/tab_syntax_error.c:2:11: syntax error
	int x =
	        ^
```

## 手動測試

### 預處理器手動測試
```bash
# 編譯編譯器
make toyc

# 測試預處理器功能
./toyc tests/fixtures/preprocessor/basic_macro.c
./toyc tests/fixtures/preprocessor/include_test.c
```

### 錯誤處理器手動測試
```bash
# 測試錯誤位置標記
./toyc tests/fixtures/error_handler/basic_syntax_error.c
./toyc tests/fixtures/error_handler/tab_syntax_error.c
./toyc tests/fixtures/error_handler/mixed_indentation_error.c
```

## 開發指南

### 新增測試
1. 在適當的 `tests/fixtures/` 子目錄中創建測試檔案
2. 在對應的測試檔案（`test_*.cpp`）中新增測試案例
3. 執行 `make test` 驗證測試通過

### 測試命名規範
- 測試檔案使用描述性名稱，如 `basic_macro.c`, `tab_syntax_error.c`
- 測試案例使用 CamelCase，如 `TestBasicMacro`, `TestTabSyntaxError`

## 開發指南

### 新增測試
1. 在適當的 `tests/fixtures/` 子目錄中創建測試檔案
2. 在對應的測試檔案（`test_*.cpp`）中新增測試案例
3. 執行 `make test` 驗證測試通過

### 測試命名規範
- 測試檔案使用描述性名稱，如 `basic_macro.c`, `tab_syntax_error.c`
- 測試案例使用 CamelCase，如 `TestBasicMacro`, `TestTabSyntaxError`

### 測試最佳實踐
- 每個測試檔案專注於單一功能或錯誤類型
- 使用有意義的註解說明測試目的
- 保持測試檔案簡潔且易於理解

## C 語法測試

C 語法測試驗證 ToyC 編譯器對標準 C 語法的支援，包含以下測試類別：

### 支援的語法特性

#### 資料類型
- **基本類型**: `bool`, `char`, `short`, `int`, `long`, `float`, `double`, `void`
- **指標類型**: 單層和多層指標 (`int*`, `int**`)
- **自定義類型**: 透過 `TYPEDEF_NAME` 支援

#### 運算符
- **算術運算符**: `+`, `-`, `*`, `/`, `%`
- **比較運算符**: `<`, `<=`, `>`, `>=`, `==`, `!=`
- **邏輯運算符**: `&&`, `||`, `!`
- **位元運算符**: `&`, `|`, `^`, `~`
- **賦值運算符**: `=`, `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `|=`, `^=`
- **遞增遞減**: `++`, `--` (前置和後置)
- **位址和解參考**: `&`, `*`
- **三元運算符**: `condition ? true_expr : false_expr`

#### 控制流
- **條件語句**: `if`, `if-else`, 巢狀條件
- **迴圈**: `for`, `while`, `do-while`, 巢狀迴圈
- **跳躍語句**: `return`

#### 函數
- **函數定義和聲明**: 包含前向聲明
- **參數**: 無參數、單參數、多參數、混合類型參數
- **可變參數**: 支援 `...` 語法
- **函數調用**: 包含遞歸調用和巢狀調用

#### 變數
- **聲明**: 單一和多個變數聲明
- **初始化**: 基本類型和指標初始化
- **作用域**: 全域、區域、區塊作用域

#### 字面值
- **整數常量**: 十進制、十六進制、八進制
- **浮點常量**: 基本浮點數和科學記號
- **字串字面值**: 包含轉義字符
- **字符常量**: 普通字符和轉義字符

### 測試檔案結構

```
tests/fixtures/syntax/
├── data_types/
│   ├── basic_types.c           # 基本資料類型
│   ├── pointer_types.c         # 指標類型
│   └── void_type.c            # void 類型
├── operators/
│   ├── arithmetic.c           # 算術運算符
│   ├── comparison.c           # 比較運算符
│   ├── logical.c              # 邏輯運算符
│   ├── bitwise.c              # 位元運算符
│   ├── assignment.c           # 賦值運算符
│   ├── increment_decrement.c  # 遞增遞減運算符
│   ├── address_dereference.c  # 位址和解參考運算符
│   └── ternary.c              # 三元運算符
├── control_flow/
│   ├── if_else.c              # if-else 語句
│   ├── for_loop.c             # for 迴圈
│   ├── while_loop.c           # while 迴圈
│   ├── do_while_loop.c        # do-while 迴圈
│   └── return_statement.c     # return 語句
├── functions/
│   ├── function_definition.c  # 函數定義和聲明
│   ├── function_parameters.c  # 函數參數
│   ├── function_calls.c       # 函數調用
│   └── variadic_functions.c   # 可變參數函數
├── variables/
│   ├── variable_declaration.c # 變數聲明
│   ├── variable_initialization.c # 變數初始化
│   └── variable_scope.c       # 變數作用域
├── literals/
│   ├── integer_literals.c     # 整數字面值
│   ├── float_literals.c       # 浮點字面值
│   └── string_literals.c      # 字串字面值
├── complex_expressions.c      # 複雜表達式
└── complete_program.c         # 完整程式範例
```

### 語法測試範例

```c
// tests/fixtures/syntax/data_types/basic_types.c
int main() {
    bool flag = true;
    char c = 'A';
    int i = 42;
    float f = 3.14;
    double d = 3.141592653589793;
    return 0;
}
```

### 手動測試語法

```bash
# 編譯編譯器
make toyc

# 測試各種語法特性
./toyc tests/fixtures/syntax/data_types/basic_types.c
./toyc tests/fixtures/syntax/operators/arithmetic.c
./toyc tests/fixtures/syntax/control_flow/for_loop.c
./toyc tests/fixtures/syntax/functions/function_calls.c
```

## 編譯器輸出測試

編譯器輸出測試驗證 ToyC 編譯器的完整編譯流程，包括 LLVM IR 生成、可執行檔案生成和程式執行結果的正確性。

### 測試層級

#### 1. LLVM IR 生成測試
驗證編譯器能夠正確生成 LLVM IR 中間表示：
- **IR 檔案生成**: 使用 `-l` 參數生成 `.ll` 檔案
- **IR 內容檢查**: 驗證生成的 IR 包含正確的函數定義、指令和結構
- **語法正確性**: 確保生成的 IR 符合 LLVM IR 語法規範

```bash
# 生成 LLVM IR
./toyc -l -o output.ll input.c

# 檢查 IR 內容
cat output.ll
```

#### 2. 可執行檔案生成測試
驗證編譯器能夠成功生成可執行的二進制檔案：
- **目標檔案生成**: 通過 LLVM 後端生成目標碼
- **連結過程**: 與系統函式庫正確連結
- **可執行檔案**: 生成可執行的二進制檔案

```bash
# 生成可執行檔案
./toyc -o output input.c

# 檢查檔案是否可執行
ls -l output
file output
```

#### 3. 程式執行結果測試
驗證生成的程式執行結果符合預期：
- **返回值檢查**: 驗證程式的退出碼 (exit code)
- **計算正確性**: 確保算術運算、函數調用等產生正確結果
- **控制流正確性**: 驗證條件判斷、迴圈等控制結構正確執行

```bash
# 執行程式並檢查退出碼
./output
echo $?  # 顯示程式退出碼
```

#### 4. 錯誤處理測試
驗證編譯器對錯誤程式碼的處理：
- **語法錯誤檢測**: 正確識別並報告語法錯誤
- **錯誤訊息**: 提供有意義的錯誤訊息
- **編譯失敗**: 在遇到錯誤時正確終止編譯過程

### 測試檔案結構

```
tests/fixtures/output/
├── simple_programs/      # 基本程式測試
│   ├── return_constant.c     # 返回常數
│   ├── return_zero.c         # 返回 0
│   └── simple_variable.c     # 簡單變數操作
├── calculations/         # 計算測試
│   ├── addition.c            # 加法運算
│   ├── multiplication.c      # 乘法運算
│   └── complex_arithmetic.c  # 複雜算術
├── functions/            # 函數測試
│   ├── simple_function.c     # 簡單函數調用
│   └── recursive_function.c  # 遞歸函數
├── control_flow/         # 控制流測試
│   ├── if_else_test.c        # if-else 測試
│   ├── for_loop_test.c       # for 迴圈測試
│   └── while_loop_test.c     # while 迴圈測試
└── error_cases/          # 錯誤情況測試
    ├── missing_semicolon.c   # 缺少分號
    ├── undefined_function.c  # 未定義函數
    └── unmatched_braces.c    # 大括號不匹配
```

### 測試範例

#### 基本計算測試
```c
// tests/fixtures/output/calculations/addition.c
int main() {
    int a = 10;
    int b = 5;
    int result = a + b;
    return result;  // 應該返回 15
}
```

#### 函數調用測試
```c
// tests/fixtures/output/functions/simple_function.c
int add(int a, int b) {
    return a + b;
}

int main() {
    int result = add(10, 5);
    return result;  // 應該返回 15
}
```

#### 遞歸函數測試
```c
// tests/fixtures/output/functions/recursive_function.c
int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

int main() {
    return factorial(5);  // 應該返回 120
}
```

### 測試執行流程

1. **編譯階段測試**
   ```bash
   # 測試 LLVM IR 生成
   ./toyc -l -o test.ll test.c

   # 測試可執行檔案生成
   ./toyc -o test_exec test.c
   ```

2. **執行階段測試**
   ```bash
   # 執行程式並檢查結果
   ./test_exec
   echo "Exit code: $?"
   ```

3. **錯誤處理測試**
   ```bash
   # 測試語法錯誤檢測
   ./toyc error_test.c  # 應該返回非零退出碼
   ```

### 自動化測試

Google Test 框架提供完整的自動化測試：

```cpp
// 測試程式執行結果
TEST_F(OutputTest, ExecutionResult_Addition) {
    std::string inputFile = "tests/fixtures/output/calculations/addition.c";
    std::string execFile = test_output_dir + "/addition";

    ASSERT_TRUE(compileFile(inputFile, execFile));
    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 15);  // 期望返回 15
}

// 測試 LLVM IR 生成
TEST_F(OutputTest, GenerateLLVMIR_ReturnConstant) {
    std::string inputFile = "tests/fixtures/output/simple_programs/return_constant.c";
    std::string llvmFile = test_output_dir + "/return_constant.ll";

    EXPECT_TRUE(generateLLVMIR(inputFile, llvmFile));
    EXPECT_TRUE(llvmIRContains(llvmFile, "define"));
    EXPECT_TRUE(llvmIRContains(llvmFile, "main"));
}
```

### 手動測試輸出

```bash
# 編譯編譯器
make toyc

# 測試基本功能
./toyc tests/fixtures/output/simple_programs/return_constant.c
./return_constant
echo $?  # 應該顯示 42

# 測試計算功能
./toyc tests/fixtures/output/calculations/addition.c
./addition
echo $?  # 應該顯示 15

# 測試函數功能
./toyc tests/fixtures/output/functions/recursive_function.c
./recursive_function
echo $?  # 應該顯示 120

# 測試 LLVM IR 生成
./toyc -l -o test.ll tests/fixtures/output/simple_programs/return_constant.c
cat test.ll  # 檢查生成的 IR

# 測試錯誤處理
./toyc tests/fixtures/output/error_cases/missing_semicolon.c  # 應該報錯
```
