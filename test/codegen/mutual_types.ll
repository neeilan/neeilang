; ModuleID = 'neeilang.main_module'
source_filename = "neeilang.main_module"

%A = type { %A*, %B* }
%B = type { %A*, %B* }

declare %A* @class_func()

declare %B* @class_func.1()