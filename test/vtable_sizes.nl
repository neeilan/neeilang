class SizeOne {
  A() : void {}
}

class SizeTwo < SizeOne {
  B() : Int { return 1; }
}

class SizeThree < SizeTwo {
  C() : void  {}
}

class StillSizeTwo < SizeTwo {
  B() : Int { return 5; }
}
