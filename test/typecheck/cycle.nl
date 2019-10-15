class Cat < Animal {
  fav_animal : Animal;
}

class Animal < Cat {
  num_legs : Int;
  name : String;
}

class Garfield < Cat {
  best_friend : Cat;
  worst_enemy : Dog;
}

class Dog < Animal { }

class CircularType < CircularType { }
