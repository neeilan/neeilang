/*
int f(); // func
int f(int x); // func
int f(int, int*); // func
int f{}; // var list-init
int f(x); // var direct-init

decltype(X) f();
decltype(X) f(int x) {} // func
decltype(X) f(int, int*); // func
decltype(X) f{}; // var list-init
decltype(X) f(x); // var direct-init

struct Foo {};
Foo f(); // func
Foo f(Foo); // func
Foo f(x); // var direct-init
*/

struct T {};

T x(int);
T x(const int&);
T x(int*);
T x(T&&);
T x(int = 5);
// T x(int());        - TODO: Parsing functions
// T x(int (*)(int)); - TODO: parsing function ptr types
//T x(auto&&);        - TODO: parsing auto