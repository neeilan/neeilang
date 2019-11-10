; ModuleID = 'neeilang.main_module'
source_filename = "neeilang.main_module"

%Parent = type { %Child* }
%Child = type { %Parent, i32 }

declare %Parent* @class_func()

declare %Child* @class_func.1()