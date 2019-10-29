class Animal {
  num_legs : Int;
  makeSound() : Void {
    "1" + "1";
    2 + 2;
    3 + "3";
    print "sound";
    print 12;
  }
}

class Cat < Animal {}

class Tree {}

var mewer : Cat;
var animal : Animal;
animal = mewer;

var tall_tree : Tree;
animal = tall_tree;
mewer = tall_tree;
