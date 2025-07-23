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

### Fixtures 資料夾結構
測試使用 fixtures 資料夾來存放測試檔案，提供更好的可讀性和維護性：

```
tests/fixtures/
├── preprocessor/          # 預處理器測試檔案
│   ├── basic_macro.c      # 基本巨集測試
│   ├── include_test.c     # include 指令測試
│   ├── conditional_*.c    # 條件編譯測試
│   └── ...
└── error_handler/         # 錯誤處理器測試檔案
    ├── basic_syntax_error.c      # 基本語法錯誤
    ├── tab_syntax_error.c        # Tab 字符錯誤
    ├── mixed_indentation_error.c # 混合縮排錯誤
    └── ...
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

### 測試最佳實踐
- 每個測試檔案專注於單一功能或錯誤類型
- 使用有意義的註解說明測試目的
- 保持測試檔案簡潔且易於理解
