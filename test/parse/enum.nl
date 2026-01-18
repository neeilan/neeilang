enum class foo : int {};

enum class bar {
    v1,
    v2 = 5,
    v3,
    v4 = 9
};

// enum class zoo : {}; # // Error:  at '{' : Expect enum underlying type
