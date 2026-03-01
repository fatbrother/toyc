# Build & Test Reference

## Make Targets

| Target | Description |
|--------|-------------|
| `make all` | Build `./toyc` compiler binary |
| `make test` | Build + run full Google Test suite |
| `make test-build` | Compile tests without running |
| `make clean` | Remove `./build/` artifacts |
| `make clean-cache` | Clear GCC output cache (`tests/gcc_output_cache/`) |
| `make format` | Auto-format all C++ with clang-format (run before every commit) |
| `make format-check` | Non-destructive format check (used by CI) |
| `make lint` | Run cpplint style checks |
| `make warn` | Build with `-Wunused-variable -Wunused-function` (used by `security.yml`) |

Build outputs land in `./build/`. Binary: `./toyc`.

## Running Tests

```bash
make test                                            # Full build + run
./build/tests/all_tests                              # Run binary directly
./build/tests/all_tests --gtest_filter="PreprocessorTest.*"  # Filter by suite
./build/tests/all_tests --gtest_filter="*keyword*"   # Filter by pattern
```

`gcc` must be in PATH — tests compile C reference programs with GCC and diff outputs.
Caches: `tests/gcc_output_cache/` and `tests/output_temp/` (both git-ignored).

## Compiler CLI

```bash
./toyc <source.c>              # Compile → a.out
./toyc <source.c> -o <name>   # Named output
./toyc <source.c> -emit-llvm   # Print LLVM IR to stdout (no binary)
```

Temp files go to `/tmp/` and are cleaned up via RAII on exit.

## CI/CD Workflows

- **`ci.yml`**: Triggered on push to `main` and PRs. Steps: install deps → `make clean && make all && make test` → validate binary → upload artifact (7-day retention).
- **`security.yml`**: Runs `make warn` (static analysis) + Valgrind memory-leak check.

> **Note**: CI installs LLVM-14 but the Makefile links against LLVM-18 (`llvm-config-18`). Keep in sync when updating CI dependencies.
