# 預處理器測試

這個目錄包含了 toyc 編譯器預處理器的測試檔案和測試資料。

## 結構

```
tests/
├── test_preprocessor.cpp        # 主要測試檔案
└── fixtures/                    # 測試資料目錄
    └── preprocessor/            # 預處理器相關測試檔案
        ├── *.c                  # 測試用的 C 原始檔案
        └── *.h                  # 測試用的標頭檔案
```

## 測試檔案說明

### 輸入檔案 (*.c)
- `basic_macro.c` - 基本物件巨集測試
- `function_macro.c` - 函數巨集測試
- `nested_macro.c` - 巢狀巨集展開測試
- `elif_test.c` - #elif 條件編譯測試
- `include_test.c` - #include 指令測試
- `include_path_test.c` - include 路徑測試
- `complex_test.c` - 複雜情境測試
- `predefined_macros.c` - 預定義巨集測試
- `empty_macro.c` - 空巨集測試
- `function_macro_multiple_params.c` - 多參數函數巨集測試
- `function_macro_no_params.c` - 無參數函數巨集測試
- `line_continuation.c` - 行連接測試
- `comments_ignored.c` - 註解處理測試
- `nested_conditionals.c` - 巢狀條件編譯測試
- `defined_operator.c` - defined() 操作符測試
- `macro_redefinition.c` - 巨集重定義測試
- `invalid_directives.c` - 無效指令測試
- `conditional_ifdef.c` - #ifdef 條件編譯測試
- `conditional_ifndef.c` - #ifndef 條件編譯測試
- `conditional_if_expression.c` - #if 表達式測試
- `conditional_else.c` - #else 條件編譯測試
- `undef_directive.c` - #undef 指令測試
- `user_defined_macros.c` - 使用者定義巨集測試

### 標頭檔案 (*.h)
- `test_header.h` - 基本標頭檔案
- `test_include.h` - 用於 include 路徑測試的標頭檔案
- `test_complex.h` - 複雜情境的標頭檔案

## 執行測試

```bash
# 編譯測試
make tests

# 執行所有測試
make run-tests

# 或直接執行
./build/tests/test_preprocessor

# 執行特定測試
./build/tests/test_preprocessor --gtest_filter="PreprocessorTest.BasicObjectMacroExpansion"
```

## 測試覆蓋範圍

測試覆蓋了以下預處理器功能：

1. **巨集展開**
   - 物件巨集 (`#define MACRO value`)
   - 函數巨集 (`#define MACRO(args) body`)
   - 巢狀巨集展開
   - 空巨集
   - 多參數函數巨集
   - 無參數函數巨集

2. **條件編譯**
   - `#ifdef` / `#ifndef`
   - `#if` 表達式
   - `#else` / `#elif`
   - `#endif`
   - 巢狀條件編譯
   - `defined()` 操作符

3. **檔案包含**
   - `#include "file.h"`
   - Include 路徑搜尋
   - 巢狀包含

4. **其他指令**
   - `#undef`
   - 預定義巨集 (`__FILE__`, `__LINE__`)
   - 行連接 (`\`)
   - 註解處理

5. **錯誤處理**
   - 無效指令
   - 缺少 `#endif`

6. **複雜情境**
   - 多個功能組合使用
   - 真實世界使用案例

所有 23 個測試都應該通過，確保預處理器功能的正確性和穩定性。
