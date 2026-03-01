# CLAUDE.md — ToyC Compiler

This file provides guidance for AI assistants working on the ToyC codebase.

## Project Overview

ToyC is an educational C compiler that transforms a significant subset of ANSI C into executable binaries. It follows a traditional multi-stage pipeline:

```
C source → Preprocessor → Lex/Flex → Bison grammar → AST → LLVM IR → Object file → Executable
```

- **Language**: C++17
- **Target language**: C (ANSI subset)
- **IR/Backend**: LLVM 18
- **Parser tools**: Flex (lexer) + Bison (parser)
- **Testing**: Google Test

## Repository Layout

```
toyc/
├── include/                  # Header files (mirrors src/ structure)
│   ├── ast/                  # AST node declarations, type system
│   ├── obj/                  # Object file generator interface
│   ├── semantic/             # Parser action interface
│   └── utility/              # Error handler, preprocessor, parse_file, raii_guard
├── src/                      # Implementation files
│   ├── ast/                  # AST nodes: expression, statement, external_definition, type, node
│   ├── obj/                  # LLVM → object file (obj_genner.cpp)
│   ├── semantic/             # Semantic actions during parsing (parser_actions.cpp)
│   ├── utility/              # error_handler, preprocessor, parse_file
│   ├── c_lexer.l             # Flex lexical analyzer
│   ├── c_grammar.y           # Bison grammar rules
│   └── toyc.cpp              # Main entry point and CLI
├── tests/                    # Google Test suite
│   ├── fixtures/             # Input files for tests
│   │   ├── error_handler/    # Error position test cases
│   │   ├── output/           # Expected program outputs
│   │   └── preprocessor/     # Macro/include test cases
│   ├── main_test.cpp         # Test runner entry point
│   ├── test_preprocessor.cpp
│   ├── test_error_handler.cpp
│   ├── test_output.cpp
│   ├── test_compiler_errors.cpp
│   └── test_type_manager.cpp
├── .clang-format             # Code style configuration
├── CPPLINT.cfg               # Lint rules (120-char line limit)
├── Makefile                  # Build system
└── README.md                 # Project documentation
```

## Build System

All build tasks go through GNU Make:

```bash
make all          # Build the compiler binary: ./toyc
make test         # Build and run all tests
make test-build   # Compile tests without running
make clean        # Remove build artifacts
make clean-cache  # Clear GCC output cache used by tests
make format       # Auto-format all C++ files with clang-format
make format-check # Check formatting compliance (non-destructive)
make lint         # Run cpplint style checks
make warn         # Build with extra warnings for unused vars/funcs
```

Build outputs go to `./build/`. The compiler binary is `./toyc`.

## Running Tests

```bash
make test                      # Full build + run
./build/tests/all_tests        # Run compiled tests directly
./build/tests/all_tests --gtest_filter="TestSuiteName.*"  # Filter
```

Tests use GCC to compile and run reference outputs, so `gcc` must be in PATH. Test caches live in `tests/gcc_output_cache/` and `tests/output_temp/` (both git-ignored).

## Using the Compiler

```bash
./toyc <source.c>              # Compile to executable (default: a.out)
./toyc <source.c> -o <output>  # Specify output name
./toyc <source.c> -emit-llvm   # Emit LLVM IR to stdout
```

Temporary files during compilation are created in `/tmp/` and cleaned up automatically via RAII.

## Namespace and Module Structure

All code lives under the `toyc::` namespace with sub-namespaces per module:

| Namespace | Header(s) | Purpose |
|-----------|-----------|---------|
| `toyc::ast` | `include/ast/*.hpp` | AST nodes, type system, codegen |
| `toyc::obj` | `include/obj/object_genner.hpp` | LLVM → object file generation |
| `toyc::semantic` | `include/semantic/parser_actions.hpp` | Semantic actions during parsing |
| `toyc::utility` | `include/utility/*.hpp` | Error handling, preprocessing, file I/O |
| `toyc` (root) | `src/toyc.cpp` | Main compiler controller and CLI |

## Code Conventions

### Naming

| Element | Convention | Example |
|---------|-----------|---------|
| AST node classes | `N` prefix + PascalCase | `NExpression`, `NBinaryOperator` |
| Other classes | PascalCase | `ASTContext`, `TypeManager` |
| Namespaces | snake_case | `toyc::ast`, `toyc::utility` |
| Methods | camelCase | `codegen()`, `allocgen()`, `getType()` |
| Constants / macros | UPPER_SNAKE_CASE | `TMP_FILE_NAME`, `QUAL_CONST` |
| Local variables | camelCase | `isSuccess`, `errorMessage` |
| Type indices | `TypeIdx` (typedef) | Index into the type table |

### Code Style

- **Standard**: C++17
- **Formatting**: Google-based clang-format (see `.clang-format`)
- **Line limit**: 120 characters (enforced by `CPPLINT.cfg`)
- **Indentation**: 4 spaces, no tabs
- **Brace style**: K&R (opening brace on same line)
- **Pointer/reference**: left-aligned — `Type* ptr`, `Type& ref`
- **Namespace indentation**: none (flat indentation inside namespaces)

Always run `make format` before committing. CI runs `make format-check`.

### Memory Management

The codebase was recently refactored to use `std::unique_ptr` throughout. Follow these rules:

- **Never use raw `new`/`delete`** for AST nodes — use `std::make_unique<>`.
- `ASTContext` owns and manages all AST node lifetimes.
- `TypeManager` lifetime is tied to `ASTContext`.
- RAII guards in `include/utility/raii_guard.hpp` handle temp file cleanup.

### AST Node Pattern

Every AST node class inherits from `NASTNode` (or a subclass) and implements:

```cpp
// Expression nodes:
virtual ExprCodegenResult codegen(ASTContext& ctx);
virtual AllocCodegenResult allocgen(ASTContext& ctx);  // lvalue address

// Statement/definition nodes:
virtual CodegenResult codegen(ASTContext& ctx);
```

Codegen methods return result types (`CodegenResult`, `ExprCodegenResult`, `AllocCodegenResult`) that carry both the LLVM value and error state. Always check for errors before using results.

### Error Handling

Use `toyc::utility::ErrorHandler` for all user-facing errors. It records source location (file, line, column) and formats error messages with source-code context. Never call `std::exit()` or `abort()` directly from AST or semantic code — propagate errors through return values or exceptions and let the top-level handle exit.

### Scope and Symbol Tables

Variables and functions are tracked via the `ScopeTable<T>` template in the semantic layer. It supports nested scopes (push/pop) and shadowing. The jump context stack handles `break`/`continue` targets for nested loops and switch statements.

## Adding New Language Features

1. **Lexer** (`src/c_lexer.l`): Add new token rules if needed.
2. **Parser** (`src/c_grammar.y`): Add grammar productions. Parser actions call into `ParserActions`.
3. **AST nodes** (`include/ast/`, `src/ast/`): Define a new `N*` class inheriting from the appropriate base. Implement `codegen()`.
4. **Semantic actions** (`src/semantic/parser_actions.cpp`): Wire up construction of the new AST node from parser reductions.
5. **Tests** (`tests/`): Add unit tests and/or output fixtures.

## Supported C Features

**Types**: `bool`, `char`, `short`, `int`, `long`, `float`, `double`, `void`, pointers (`*`, multi-level), `struct` (member access via `.` and `->`), type qualifiers `const`/`volatile` (parsed but not enforced semantically).

**Operators**: Arithmetic, comparison, logical, bitwise (including compound assignments `<<=`, `>>=`, etc.), `++`/`--` (prefix/postfix), ternary `?:`, address-of `&`, dereference `*`, cast `(type)`, `sizeof`.

**Control flow**: `if`/`else`, `switch`/`case`/`default`, `for`, `while`, `do-while`, `return`, `break`, `continue`, `goto`.

**Functions**: Definitions, forward declarations, variadic (`...`), recursion.

**Not yet implemented**: `typedef`, `enum`, `union`, bit-fields, K&R-style function definitions, complex function-pointer declarations.

## CI/CD

Two GitHub Actions workflows:

- **`ci.yml`**: Triggered on push to `main` and pull requests. Installs dependencies (GCC, Flex, Bison, LLVM, GTest), runs `make clean && make all && make test`, validates the binary, and uploads build artifacts (7-day retention).
- **`security.yml`**: Runs `make warn` (static analysis), Valgrind memory-leak detection, and reports a summary.

Note: CI currently uses LLVM-14 while the Makefile targets LLVM-18. Keep this in sync when updating dependencies.

## Commit Message Style

Follow Conventional Commits:

```
feat:     New language feature or capability
fix:      Bug fix
test:     Add or update tests
refactor: Code restructuring without behavior change
chore:    Build, tooling, or CI changes
docs:     Documentation only
```

Examples from project history: `feat: implement qualified type handling (const/volatile)`, `fix: add clone method to avoid double delete`, `refactor: use Smart Pointers to manage AST Node`.

## Key Files Quick Reference

| File | What to look at |
|------|----------------|
| `src/toyc.cpp` | Compiler entry point, CLI, orchestration |
| `src/c_grammar.y` | All grammar rules and parser actions |
| `src/c_lexer.l` | Token definitions |
| `include/ast/node.hpp` | Base AST node and ASTContext |
| `include/ast/type.hpp` | Type system, TypeIdx, TypeManager |
| `include/utility/error_handler.hpp` | Error reporting API |
| `Makefile` | All build targets and flags |
| `README.md` | User-facing documentation |
