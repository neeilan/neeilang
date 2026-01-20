int foo(int x) {
    return 0;
}

static constexpr int bar(int x) {
    return 0;
}

virtual static inline friend noexcept int bar(int x) {
    return 0;
}
