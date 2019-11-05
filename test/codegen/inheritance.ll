; ModuleID = 'neeilang.main_module'
source_filename = "neeilang.main_module"

%A = type { i32, double }
%B = type { i32, double, i32, double }
%B1 = type { i32, double, i32, double }
%C = type { i32, double, i32, double, i32, double }

declare %A @class_func()

declare %B @class_func.1()

declare %B1 @class_func.2()

declare %C @class_func.3()