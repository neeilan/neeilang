static_assert(1 == 1);
static_assert((1 == 1));

namespace x {
static_assert(1 == 1);
}

void Bar() {
    static_assert(1 == 1);
}

struct Foo {
    static_assert(1 == 1);
};