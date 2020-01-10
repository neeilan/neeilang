;CODEGEN
; ModuleID = 'neeilang.main_module'
source_filename = "neeilang.main_module"

%SizeOne = type { i8, i8* }
%SizeTwo = type { %SizeOne, i8, i8* }
%SizeThree = type { %SizeTwo, i8, i8* }
%StillSizeTwo = type { %SizeTwo, i8, i8* }

@__vtable_SizeOne = external global [1 x i8*]
@__vtable_SizeTwo = external global [2 x i8*]
@__vtable_SizeThree = external global [3 x i8*]
@__vtable_StillSizeTwo = external global [2 x i8*]

declare i32 @printf(i8*, ...)

define void @SizeOne_A(%SizeOne* %this) {
entry:
  %this1 = alloca %SizeOne*
  store %SizeOne* %this, %SizeOne** %this1
}

define i32 @SizeTwo_B(%SizeTwo* %this) {
entry:
  %this1 = alloca %SizeTwo*
  store %SizeTwo* %this, %SizeTwo** %this1
  ret i32 1
}

define void @SizeThree_C(%SizeThree* %this) {
entry:
  %this1 = alloca %SizeThree*
  store %SizeThree* %this, %SizeThree** %this1
}

define i32 @StillSizeTwo_B(%StillSizeTwo* %this) {
entry:
  %this1 = alloca %StillSizeTwo*
  store %StillSizeTwo* %this, %StillSizeTwo** %this1
  ret i32 5
}
