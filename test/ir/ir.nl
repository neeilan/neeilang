int foo() {
    return 1 + 2 + 3;
}

template <typename T>
class optional {
    using unique_vals = T::enum_values::size;
    T value;
    bool engaged = true;
};

optional<int> myOpt;

/*
namespace n1 {
    int foo() {
        return 3 + 4;
    }
}*/