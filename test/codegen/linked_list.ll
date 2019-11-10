; ModuleID = 'neeilang.main_module'
source_filename = "neeilang.main_module"

%LLNode = type { i32, i1, %LLNode* }

declare %LLNode* @class_func()