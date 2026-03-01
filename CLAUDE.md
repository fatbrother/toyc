# CLAUDE.md — ToyC Compiler

ToyC is an educational C compiler: C source → Preprocessor → Flex/Bison → AST → LLVM 18 IR → executable.

- **Language**: C++17 | **Parser**: Flex + Bison | **Tests**: Google Test

## Repository Layout

```
toyc/
├── include/          # Headers (ast/, obj/, semantic/, utility/)
├── src/              # Implementations + c_lexer.l, c_grammar.y, toyc.cpp
├── tests/            # GTest suite + fixtures/
├── .claude/commands/ # Project skills (invoke with /skill-name)
├── Makefile
└── README.md
```

## Essential Commands

```bash
make all              # Build ./toyc
make test             # Build + run all tests
make format           # Auto-format (run before every commit)
make clean            # Remove build artifacts
./toyc <src.c>        # Compile to a.out
./toyc <src.c> -o out # Named output
./toyc <src.c> -emit-llvm  # Dump LLVM IR
```

## Commit Style (Conventional Commits)

```
feat: | fix: | test: | refactor: | chore: | docs:
```

## Key Files

| File | Purpose |
|------|---------|
| `src/toyc.cpp` | Entry point, CLI |
| `src/c_grammar.y` | All grammar rules |
| `src/c_lexer.l` | Token definitions |
| `include/ast/node.hpp` | Base AST node, ASTContext |
| `include/ast/type.hpp` | Type system, TypeIdx, TypeManager |
| `include/utility/error_handler.hpp` | Error reporting API |

## Available Skills

Skills live in `.claude/commands/` and are loaded with `/skill-name`. Only invoke the skills below directly — other skills (e.g. `simplify`, `session-start-hook`) are system skills used internally by Claude and should not be called by users.

| Skill | When to use |
|-------|-------------|
| `/build` | Make targets, test filtering, compiler flags |
| `/conventions` | Naming rules, style, memory management, error handling |
