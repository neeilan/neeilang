template <typename T>
T Test(T arg) {
    float a = 0.1;
    T b = 4;
    return arg * 2;
}

int f = Test<int>(3);