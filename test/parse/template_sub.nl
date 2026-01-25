template <typename T, typename F>
T Test(T arg) {
    F a = 0.1;
    T b = 4;
    return arg * 2;
}

int f = Test<int, float>(3);

struct TRT {
    using key_type = int;
    using value_type = float;
};

template <typename Traits>
struct Foo {
    int w;
    // TRT::value_type y; - TODO (isType for somethng like TRT::value_type requires declctx lookup)
    typename Traits::key_type x; // dependent

    using TypeT = Traits::key_type;
    // IMPORTANT:
    // Test - struct Foo's context, but
    // `Traits::key_type` (after substitution) must be looked up in the
    // template argument context!
    using FuncT = Test<Traits::key_type, Traits::value_type>;

    Traits getValue() {
        return 42;
    }

};

::Foo<TRT> x;
Foo<TRT>::TypeT y;