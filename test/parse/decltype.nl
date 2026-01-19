// Examples modified from https://en.cppreference.com/w/cpp/language/decltype.html
struct A { float x; };
const A* a = nil;
 
decltype(a->x) y;       // type of y is float (declared type)
decltype((a->x)) z;     // type of z is const double& (lvalue expression)

template<typename T, typename U>
decltype(t + u) add(T t, U u) {    // return type depends on template parameters
    return t + u;                  // return type can be deduced since C++14
}

template <typename I, typename J>
inline constexpr bool is_same_v = true;

static_assert(is_same_v<decltype(i), decltype(j)>);