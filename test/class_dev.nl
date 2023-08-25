// 'Scratch' test file for checking implementation of classes during
// x86 backend development. Should be removed in favor of test/functional
// tests once the feature is reasonably stable.

class Animal {
  name : Int;
  age : Int;

  init() { print "Animal init called"; return this; }
  sayHi(x : Int) : Int { print "hi"; return 3; }

}

fn main() : Int {
	var a = Animal.init();
	a.sayHi(4);
	return 1;
}
