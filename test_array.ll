; ModuleID = 'toyc'
source_filename = "toyc"

@string_literal = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %arr = alloca [5 x i32], align 4
  %arr_decay = getelementptr [5 x i32], ptr %arr, i32 0, i32 0
  %arrayidx = getelementptr i32, ptr %arr_decay, i32 4
  %arrayelem = load i32, ptr %arrayidx, align 4
  %arr_decay1 = getelementptr [5 x i32], ptr %arr, i32 0, i32 0
  %arrayidx2 = getelementptr i32, ptr %arr_decay1, i32 0
  %arrayelem3 = load i32, ptr %arrayidx2, align 4
  %add = add i32 %arrayelem3, %arrayelem
  %0 = call i32 (ptr, ...) @printf(ptr @string_literal, i32 %add)
  ret i32 0
}
