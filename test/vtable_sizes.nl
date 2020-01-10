class SizeOne {
  A() : Void {}
}

class SizeTwo < SizeOne {
  B() : Int { return 1; }
}

class SizeThree < SizeTwo {
  C() : Void  {}
}

class StillSizeTwo < SizeTwo {
  B() : Int { return 5; }
}
