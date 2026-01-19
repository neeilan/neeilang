// Examples modified from https://en.cppreference.com/w/cpp/language/decltype.html
struct A { var x : float; };
var a : const A* = nil;
 
var y : decltype(a->x);       // type of y is float (declared type)
var z : decltype((a->x));     // type of z is const double& (lvalue expression)

template<typename T, typename U>
fn add(t: T, u: U) : decltype(t + u) { // return type depends on template parameters
    return t + u;                      // return type can be deduced since C++14
}

template <typename I, typename J>
inline constexpr var is_same_v : bool = true;

static_assert(is_same_v<decltype(i), decltype(j)>);