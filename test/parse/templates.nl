namespace foo::bar {
template <typename T>
fn Test(arg : T) : T {
    return arg * 2;
}
}

template <typename T, typename C>
fn foo(arg : x) : T {
    return T * x + C;
}

template <typename T>
struct Animal {
  var name : std::string;
  var age : T;

  fn init() { print "Animal init called"; return this; }
  fn sayHi(x : int, y : T) : int { print "hi"; return 3 + y; }
};

template <typename... T, typename C>
fn variadic(arg : T...) : T {
    return 3;
}

var res : foo::bar::Test<int, float> = 3;
var myAnimal : ::Animal< const int> = 5;
var noArgAnimal : ::Animal<> = 5;

// This is a Template Parsing Ambiguity and requires parser type tracking
var res2 = foo::Test<int>(3);
var res3 = x < z > (y);

namespace n1 {
template <typename T>
fn qux() : void {}
class cho {
    fn init() { return this; }
};
}
namespace n2 {
var qux : int;
var cho : int  = 42;
}

// TODO: These two get parsed identically, but they shouldn't
// The parser should be name[space]-aware
var res4 = n1::qux < &const n1::y > (y);
var res5 = n2::qux < n2::y > (y);