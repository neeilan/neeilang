; ModuleID = 'neeilang.main_module'
source_filename = "neeilang.main_module"

define void @fnWithLocalVars() {
entry:
  %myFloat = alloca double
  %myBool = alloca i1
  %myInt = alloca i32
  store i32 4, i32* %myInt
  store i1 true, i1* %myBool
  store double 0.000000e+00, double* %myFloat
}