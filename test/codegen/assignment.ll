; ModuleID = 'neeilang.main_module'
source_filename = "neeilang.main_module"

define void @fnWithLocalVars() {
entry:
  %yourInt = alloca i32
  %myInt = alloca i32
  store i32 4, i32* %myInt
  store i32 5, i32* %yourInt
  %myInt1 = load i32, i32* %myInt
  store i32 %myInt1, i32* %yourInt
}