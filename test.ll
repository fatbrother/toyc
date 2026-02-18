; ModuleID = 'toyc'
source_filename = "toyc"

@string_literal = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @sum_vla(i32 %n, ptr %arr) {
entry:
  %n1 = alloca i32, align 4
  store i32 %n, ptr %n1, align 4
  %arr2 = alloca ptr, align 8
  store ptr %arr, ptr %arr2, align 8
  %sum = alloca i32, align 4
  store i32 0, ptr %sum, align 4
  %i = alloca i32, align 4
  store i32 0, ptr %i, align 4
  br label %for_condition

for_after:                                        ; preds = %for_condition
  %sum9 = load i32, ptr %sum, align 4
  ret i32 %sum9

for_condition:                                    ; preds = %for_increment, %entry
  %n7 = load i32, ptr %n1, align 4
  %i8 = load i32, ptr %i, align 4
  %lt = icmp slt i32 %i8, %n7
  br i1 %lt, label %for_body, label %for_after

for_increment:                                    ; preds = %for_body
  %i5 = load i32, ptr %i, align 4
  %add6 = add i32 %i5, 1
  store i32 %add6, ptr %i, align 4
  br label %for_condition

for_body:                                         ; preds = %for_condition
  %i3 = load i32, ptr %i, align 4
  %ptridx = getelementptr i32, ptr %arr2, i32 %i3
  %arrayelem = load i32, ptr %ptridx, align 4
  %sum4 = load i32, ptr %sum, align 4
  %add = add i32 %sum4, %arrayelem
  store i32 %add, ptr %sum, align 4
  br label %for_increment
}

define i32 @main() {
entry:
  %size = alloca i32, align 4
  store i32 5, ptr %size, align 4
  %size1 = load i32, ptr %size, align 4
  %numbers.vla = alloca i32, i32 %size1, align 4
  %i = alloca i32, align 4
  store i32 0, ptr %i, align 4
  br label %for_condition

for_after:                                        ; preds = %for_condition
  %size7 = load i32, ptr %size, align 4
  %0 = call i32 @sum_vla(i32 %size7, ptr %numbers.vla)
  %1 = call i32 (ptr, ...) @printf(ptr @string_literal, i32 %0)
  ret i32 0

for_condition:                                    ; preds = %for_increment, %entry
  %size5 = load i32, ptr %size, align 4
  %i6 = load i32, ptr %i, align 4
  %lt = icmp slt i32 %i6, %size5
  br i1 %lt, label %for_body, label %for_after

for_increment:                                    ; preds = %for_body
  %i4 = load i32, ptr %i, align 4
  %add = add i32 %i4, 1
  store i32 %add, ptr %i, align 4
  br label %for_condition

for_body:                                         ; preds = %for_condition
  %i2 = load i32, ptr %i, align 4
  %ptridx = getelementptr i32, ptr %numbers.vla, i32 %i2
  %i3 = load i32, ptr %i, align 4
  %mul = mul i32 %i3, 3
  store i32 %mul, ptr %ptridx, align 4
  br label %for_increment
}
