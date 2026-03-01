---
name: conventions
description: Code conventions for ToyC — naming, style, memory management, error handling, and scope tables. Load automatically when writing, reviewing, or explaining ToyC code.
user-invocable: false
---

# Code Conventions

## Naming

| Element | Convention | Example |
|---------|-----------|---------|
| AST node classes | `N` prefix + PascalCase | `NExpression`, `NBinaryOperator` |
| Other classes | PascalCase | `ASTContext`, `TypeManager` |
| Namespaces | snake_case | `toyc::ast`, `toyc::utility` |
| Methods | camelCase | `codegen()`, `allocgen()`, `getType()` |
| Constants / macros | UPPER_SNAKE_CASE | `TMP_FILE_NAME`, `QUAL_CONST` |
| Local variables | camelCase | `isSuccess`, `errorMessage` |
| Type indices | `TypeIdx` typedef | Index into the type table |

## Code Style

- **Standard**: C++17
- **Formatting**: Google-based clang-format — run `make format` before every commit
- **Line limit**: 120 characters (enforced by `CPPLINT.cfg`)
- **Indentation**: 4 spaces, no tabs
- **Braces**: K&R — opening brace on the same line
- **Pointers/references**: left-aligned — `Type* ptr`, `Type& ref`
- **Namespace indentation**: none (flat inside namespaces)

## Memory Management

The codebase uses `std::unique_ptr` throughout (refactored from raw pointers).

- **Never** use raw `new`/`delete` for AST nodes — use `std::make_unique<>`.
- `ASTContext` owns all AST node lifetimes.
- `TypeManager` lifetime is tied to `ASTContext`.
- Temp file cleanup uses RAII guards (`include/utility/raii_guard.hpp`).

## Error Handling

- Use `toyc::utility::ErrorHandler` for all user-facing errors.
- It records file, line, column and formats messages with source-code context.
- **Never** call `std::exit()` or `abort()` from AST or semantic code.
- Propagate errors via return values (`CodegenResult` etc.) and let `src/toyc.cpp` handle exit.

## Scope & Symbol Tables

- Variables and functions tracked via `ScopeTable<T>` template (push/pop, shadowing).
- Jump targets (`break`/`continue`) tracked via a jump context stack for nested loops and switches.
