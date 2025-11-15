; ModuleID = 'toyc'
source_filename = "toyc"

@string_literal = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %arr = alloca [5 x i32], align 4
  %arr_elem_0 = getelementptr [5 x i32], ptr %arr, i32 0, i32 0
  store i32 1, ptr %arr_elem_0, align 4
  %arr_elem_1 = getelementptr [5 x i32], ptr %arr, i32 0, i32 1
  store i32 2, ptr %arr_elem_1, align 4
  %arr_elem_2 = getelementptr [5 x i32], ptr %arr, i32 0, i32 2
  store i32 3, ptr %arr_elem_2, align 4
  %arr_elem_3 = getelementptr [5 x i32], ptr %arr, i32 0, i32 3
  store i32 4, ptr %arr_elem_3, align 4
  %arr_elem_4 = getelementptr [5 x i32], ptr %arr, i32 0, i32 4
  store i32 5, ptr %arr_elem_4, align 4
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
