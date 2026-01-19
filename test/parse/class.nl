class Foo {
    static_assert(1 == 1);

    // namespace bar {}; // - Invalid class member

    int x = 5;

    Foo Foo() {}

    enum class Color { RED, GREEN };

    class Bar {
        int x;
    };

    template <typename T>
    T add(T a, T b) {
        return a + b;
    }

    template <typename X>
    struct Holder {
        const X& item;
    };

    using MyStr = std::string;

    template <typename T>
    class string {};
    using WideStr = string<eWide>;

    using MyStr = std::string;
};