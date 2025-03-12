# C Compiler

This is a simple C compiler written with llvm and yacc.

## How to use

To compile the compiler, run the following command:

```bash
make
```

To compile a C file, run the following command:

```bash
./toyc <input_file> <output_file>
```

## Grammar

The grammar of the C language is based on the [ANSI-C-grammar](https://www.lysator.liu.se/c/ANSI-C-grammar-y.html).