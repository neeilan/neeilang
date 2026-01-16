!x;
-x;
&x;
*x;

!x + !x;
-1 + -1;
&x + &x;
*x + *x;
*x * *x;

// Ref: https://godbolt.org/z/v96ff8MPM
**x;
*&x;
&(*&x);
*&(*&x);
// Note: Address of address &(&x) is never legal, so we don't bother parsing it, as it can be logical and (&&)

// Ref: https://godbolt.org/z/5GG63WMv6
--x;
++x;
----x;
++++x;
++--x;
----x;
--x + 5;