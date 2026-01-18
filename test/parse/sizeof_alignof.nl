class Foo {};
sizeof(Foo);
sizeof(sizeof(Foo));
sizeof(sizeof(Foo*));
sizeof(sizeof(&Foo));
static_assert(sizeof(Foo) == 1);

sizeof(x);
static_assert(sizeof(x) == 1);
static_assert(sizeof(&x) == 1);

sizeof(4 + 2);

alignof(Foo);
static_assert(alignof(Foo) == 1);
alignof(Bar*);
alignof(&Bar*);

template <typename T>
class Bar {};
alignof(Bar<int>);