// Modulus precedence and chaining
// Ref: https://godbolt.org/z/eTr1f4TWd
7 % 4 == 1;
3 % 2 & 2;
3 % 2 - 5;
3 % 2 * 11 % 6;
3 % 2 + 5;
3 % ++x;
7 % 4 % 2;