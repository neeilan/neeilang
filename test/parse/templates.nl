namespace foo::bar {
    template <typename T>
    T Test(T arg) {
        return arg * 2;
    }
} // namespace

template <typename T, typename C>
T foo(int x) {
    return T * x + C;
}

template <typename... T, typename C>
T variadic(T... arg) {
    return 3;
}

foo::bar::Test<int, float>  res = 3;

struct string{}; // no std::string
template <typename T>
struct Animal {
//  string name;
//  T age;

//  Animal Animal() { print "Animal init called"; return this; }
//  int sayHi(int x, T y) { print "hi"; return 3 + y; }
};

::Animal<int> myAnimal = 5;
::Animal<> noArgAnimal = 5;

foo::Test<int> res2 = 3;
int res3 = x < z > (y);


namespace n1 {
template <typename T>
void qux() {}
class cho {
    cho cho() { return this; }
};
}
namespace n2 {
int qux;
int cho = 42;
}

// TODO: These two get parsed identically, but they shouldn't
// The parser should be name[space]-aware
int res4 = n1::qux < const n1::y& > (y);
int res5 = n2::qux < n2::y > (y);