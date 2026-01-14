class SizeOne {
  A() : void {}
}

class SizeTwo < SizeOne {
  B() : int { return 1; }
}

class SizeThree < SizeTwo {
  C() : void  {}
}

class StillSizeTwo < SizeTwo {
  B() : int { return 5; }
}
