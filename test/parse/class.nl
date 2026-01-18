class Foo {
    static_assert(1 == 1);

    // namespace bar {}; // - Invalid class member

    var x : int = 5;

    fn Foo() : Foo {}

    enum class Color { RED, GREEN };

    class Bar {
        var x : int;
    };

    template <typename T>
    fn add(a : T, b : T) : T {
        return a + b;
    }

    template <typename T>
    struct Holder {
        var item : T;
    };

};