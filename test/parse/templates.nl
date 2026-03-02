namespace foo::bar {

    template <typename T>
    int TestNoTempl(int arg) {
        return arg * 2;
    }

    template <typename T>
    T Test(T arg) {
        return arg * 2;
    }

    template <typename T>
    struct Qux {T member;};
} // namespace

template <typename T, typename C>
T foo2(int x) {
    return T * x + C;
}

template <typename... T, typename C>
T variadic(T... arg) {
    return 3;
}

// foo::bar::Test<int, float>  res = 3;


struct string{}; // no std::string


template <typename T>
struct Animal {
  string name;
  T age;

//  Animal Animal() { print "Animal init called"; return this; } // TODO: parse constructors
  int sayHi(int x, T y) { print "hi"; return 3 + y; }
};



Animal myAnimal = 5;

Animal<int> myAnimal = 5;
//foo::Animal<int> myAnimal = 5;


::Animal<int> myAnimal = 5;


::Animal<> noArgAnimal = 5;


//foo::bar::Test<int> res2 = 3;  // error because foo::bar::Test<int> is a function
auto res = foo::bar::Test<int>();  // ok
foo::bar::Qux res2 = 3;  // ok because foo::bar::Qux<int> is a class tmpl
foo::bar::Qux<int> res2 = 3;  // ok because foo::bar::Qux<int> is a class


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
// The parser should be name[space]-aware. UPDATE: NOW IT IS!!! // Next step: Name lookup for template args themselves (const n1::y& doesn't actually exist)
int res4 = n1::qux < const n1::y& > (y);
int res5 = n2::qux < n2::y > (y);