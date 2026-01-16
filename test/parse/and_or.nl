a and b;
a or b;

a & b;
a | b;

a && b;
a || b;

// In C++,  1 & 2 | 2 is parsed as (1 & 2) | 2
// See https://godbolt.org/z/rMMKn7q9T
1 & 2 | 2;    // <Expr (| (& 1 2) 2)/>
1 | 2 & 2;    // <Expr (| 1 (& 2 2))/>

// In C++, 1 && 2 | 2 is parsed as 1 && (2 | 2)
// See https://godbolt.org/z/zMd3qKjYK
1 && 2 | 2;   // <Expr (&& 1 (| 2 2))/>