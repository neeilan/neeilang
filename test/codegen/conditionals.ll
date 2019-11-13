; ModuleID = 'neeilang.main_module'
source_filename = "neeilang.main_module"

define i32 @cond1(i32) {
entry:
  %a1 = alloca double
  store double 1.600000e+01, double* %a1
  %a11 = load double, double* %a1
  %cmp_lt_tmp = fcmp ult double %a11, 5.100000e+00
  %ifcond = icmp ne i1 %cmp_lt_tmp, false
  br i1 %ifcond, label %then, label %else

then:                                             ; preds = %entry
  ret i32 1
  br label %ifcont

else:                                             ; preds = %entry
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  ret i32 3
}