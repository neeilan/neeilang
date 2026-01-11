template <typename T>
fn Test(arg : T) : T {
    return T * 2;
}

template <typename T, typename C>
fn foo(arg : x) : T {
    return T * x + C;
}