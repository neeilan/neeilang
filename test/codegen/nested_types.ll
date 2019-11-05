; ModuleID = 'neeilang.main_module'
source_filename = "neeilang.main_module"

%A = type { i32, double }
%B = type { i32, %A, double }

declare %A @class_func()

declare %B @class_func.1()