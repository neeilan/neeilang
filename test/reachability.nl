fn Test1() : Int {
  if (5 > 1) {
    return 5;
  } else {
    if (7 < 14) {
      return 6;
    } else {
      return 5;
    }
  }
  4 + 9;    // [line 11] Error:  at ';' : Unreachable statement 
  return 7; // [line 12] Error:  at 'return' : Unreachable statement 
}


class MyClass {
  init() {
    return 3; // Type checker should catch this..
  }
}
