namespace foo::bar {
template <typename T>
fn Test(arg : T) : T {
    return T * 2;
}
}

template <typename T, typename C>
fn foo(arg : x) : T {
    return T * x + C;
}

template <typename T>
struct Animal {
  name : Int;
  age : Int;

  init() { print "Animal init called"; return this; }
  sayHi(x : Int, y : T) : Int { print "hi"; return 3 + y; }

}