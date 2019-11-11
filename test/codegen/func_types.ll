; ModuleID = 'neeilang.main_module'
source_filename = "neeilang.main_module"

%A = type { i32 }
%B = type { %A, i1 }

declare %A* @class_func()

declare %B* @class_func.1()

define i32 @myFunc(%A*, %B*, i32, i32) {
entry:
  ret i32 3
}