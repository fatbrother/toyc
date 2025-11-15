; ModuleID = 'toyc'
source_filename = "toyc"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@string_literal = private unnamed_addr constant [37 x i8] c"Integer to Integer Casts Result: %d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %result = alloca i32, align 4
  store i32 0, ptr %result, align 4
  %i = alloca i32, align 4
  store i32 300, ptr %i, align 4
  %c = alloca i8, align 1
  %i1 = load i32, ptr %i, align 4
  %to_char = trunc i32 %i1 to i8
  store i8 %to_char, ptr %c, align 1
  %s = alloca i16, align 2
  %i2 = load i32, ptr %i, align 4
  %to_short = trunc i32 %i2 to i16
  store i16 %to_short, ptr %s, align 2
  %l = alloca i64, align 8
  %i3 = load i32, ptr %i, align 4
  %to_long = sext i32 %i3 to i64
  store i64 %to_long, ptr %l, align 8
  %c4 = load i8, ptr %c, align 1
  %to_int = sext i8 %c4 to i32
  %result5 = load i32, ptr %result, align 4
  %add = add i32 %result5, %to_int
  store i32 %add, ptr %result, align 4
  %s6 = load i16, ptr %s, align 2
  %to_int7 = sext i16 %s6 to i32
  %result8 = load i32, ptr %result, align 4
  %add9 = add i32 %result8, %to_int7
  store i32 %add9, ptr %result, align 4
  %l10 = load i64, ptr %l, align 8
  %to_int11 = trunc i64 %l10 to i32
  %result12 = load i32, ptr %result, align 4
  %add13 = add i32 %result12, %to_int11
  store i32 %add13, ptr %result, align 4
  %result14 = load i32, ptr %result, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @string_literal, i32 %result14)
  %result15 = load i32, ptr %result, align 4
  ret i32 %result15
}
