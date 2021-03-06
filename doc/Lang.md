NeeiLang Language Overview
==========================

### Reserved Keywords

  true   false   lambda   else   if 
  for    while   super    this   fn 
  class  print   var


### Introduction

Every valid construct in NL is either a statement or an expression.
Each expression evaluates to a NL value, which has a type. There 
are six primitive types:

|  Type  | Description                                        |
|--------|-----------------------------------------------------
| Int    | Signed integer >= 32 bits wide                     |
| Float  | 'At-least' single-precision FP number              |
| Bool   | Boolean type - true or false                       |
| String | Character sequence - each char is >= 8 bits wide   |
| Array  | An array of type T and size s is indicated as T[s] |
| Void   | Indicates no return value in function signatures   |


A NL program is a sequence of function and/or class statements.

Functions are declared using the `fn` keyword. NL programs start 
executing in the `main() : Int` function. Hence, this is the 
simplest possible 'Hello, world!' program:

```
fn main() : Int {              
  print "Hello, world!";        
  return 0;                     
}                              
```

NL is a statically-typed language, and the types of parameters 
and return values must be explicitly declared:

```
fn add(a : Float, b : Float) : Float { return a + b; }
```

While this adds verbosity, it also eliminates an entire class of
type errors at compile time. For example, calling `add(1.5, "two")`
fails with the following message:

```
Expected argument type ( Float Float ) but got ( Float String )
```

Similarly, variables are strongly typed and require explicit types:
```
var age  : Int;
var name : String;
```

However, if assigned an initial value, the compiler can infer the 
type of a variable:
```
var age = 5;  // Inferred as Int
```


### Classes & Objects

In addition to primitive types, programmers can also define their own 
types via classes. A class consists of zero or more fields and/or
methods.

```
class Animal {
  numLegs : Int;
  
  init(legs: Int) {
    this.numLegs = legs;
    return this;
  }
  
  makeSound() : Void { print "grr!"; }
}
```

Objects of a class are created using the special `init` method:
```
var pet = Animal.init(4);
```

In methods, the `this` keyword is used to refer to the callee object.
Note that methods within a class do not have the `fn` keyword. The 
`init` method also does not require a return type, as it  always 
returns a new instance of the enclosing class.
   
The `<` symbol is used to create a subclass.
```
class Cat < Animal {
  init() { return this; }
  makeSound() : Void { print "meeeow!"; }
}
```


### Polymorphism

The type system supports polymorphic variable assignment, function 
argument passing, and function returns. Consider the following (legal) code:
```
class Super { flag : Bool; init() { return this; } }
class Sub < Super { init() { return this; } }

fn returnsSuper(superArg : Super) : Super {
  var superLocal : Super = Super.init();
  
  if (superArg.flag) {
    return superLocal;
  } else {
     return Sub.init();
  }
}

// In main(): 
returnsSuper( Super.init() );
returnsSuper( Sub.init() );
```

NeeiLang also performs dynamic single-dispatch in method calls.
```
var a : Animal = Cat.init();
a.makeSound(); // prints 'meeeow!'
```


### Control Flow

if/else and loop syntax is almost identical to other C-like languages.
```
if (getBool() && getAnotherBool()) {
  doThing();
} else {
  doOtherThing();
}

while (i < 5) {
  if (i == 0) { print "zero!";}
  else { print i; }
}

for (var i = 1; i <= 5; i = i+1) {
  print "I can count to 5!";
}
```


### Print statement

To print a few primitive types (most notably Strings) to stdout, the
print statement may be used.
```
print expr;
print 3.14;
print "hello";
```


### Comments

C++ style single and multi-line comments are supported.
```
// Single, and
/*
Multi-line
comments
*/
```


### Type semantics

Types in NL can be widely divided into numeric and non-numeric types.
Numeric types are the primitive `Int`, `Float`, and  `Bool` types.
Almost every other type, including user-defined types, is non-numeric.
The general rule for value vs reference semantics is as follows:

"Numeric types exhibit value semantics; all other types exhibit reference semantics."

As an implementation note, objects are always heap-allocated (à la Java),
so user-defined value-types do not exist in NL. Unreachable objects are 
reclaimed by the garbage collector.


### More examples

There is an extensive test-suite, annotated with expected outputs 
using splat [1], in the tests/ directory [2]. If you find the above 
examples too trivial, the test programs may be more interesting.


### Resources  
  
[1] https://github.com/neeilan/splat

[2] https://github.com/neeilan/neeilang/tree/master/test
  
