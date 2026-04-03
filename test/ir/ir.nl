template <typename T>
class optional {
    // using unique_vals = T::enum_values::size;
    // static constexpr int WIDTH = sizeof(T);
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
    optional< optional<float> > myOpt2;
    myOpt = 42;
}

