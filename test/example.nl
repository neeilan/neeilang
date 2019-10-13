class Animal {}

class Cat < Animal {
  sayHi() : String {}
  sayHi(name : String) : String {}
}


class Octopus < Animal {
  tentacles : Int = 8;
  color : String = "white";

  sayHi() : String {}
  sayHi(name : String) : String {}
}

class Dog < Animal {}
fn funcName(intArg : Int, strArg : String) : void {
  return intArg + strArg;
}

fn doStuff() : Void {
  return;
}

fn main() : Int {
  doStuff();
  return 10;
}

fn sayHi(first : String, last : String, sayHi : String) : int {
  print "Hi, " + first + " " + last + "!";
  return 0;
}

var i : Int = sayHi("Dear", "Reader");
