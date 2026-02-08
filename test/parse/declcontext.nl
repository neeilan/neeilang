int foo() { return 0; }

class bar {
    static int foo() { return -1; }
};

namespace ns1 {
    int foo() { return 1; }

    namespace ns2 {
        int foo() { return 12; }
        class bar {};
        enum class baz { a = 1, b = 2 };
    }
}

template <typename T>
int tFoo(T x) { return 3; }