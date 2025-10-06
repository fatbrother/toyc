// 字串字面值測試
int main() {
    // 基本字串
    char *str1 = "Hello, World!";
    char *str2 = "ToyC Compiler";
    
    // 空字串
    char *empty = "";
    
    // 包含轉義字符的字串
    char *escaped = "Line 1\nLine 2\tTabbed";
    char *quotes = "He said \"Hello\"";
    char *backslash = "Path\\to\\file";
    
    // 長字串
    char *long_str = "This is a very long string that contains multiple words and should be parsed correctly by the lexer.";
    
    return 0;
}