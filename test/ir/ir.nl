int foo() {
    return 1 + 2 + 3;
}

template <typename T>
class optional {
    using unique_vals = T::enum_values::size;
    T value;
    bool engaged = true;

    optional<T>& operator=(T value) {
        this->value = value;
        return *this;
    }
};

template class optional<int>;

int main() {
    optional<int> myOpt;
    myOpt = 42;
}

/*
namespace n1 {
    int foo() {
        return 3 + 4;
    }
}*/