static_assert(1 == 1);
static_assert((1 == 1));

namespace x {
static_assert(1 == 1);
}

fn Bar() : void {
    static_assert(1 == 1);
}

// TODO: Support inside class declaration
// struct Foo {
//    static_assert(1 == 1);
// }