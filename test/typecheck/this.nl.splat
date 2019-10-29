class SomeObject {
  someMethod() : Void {
    var a : SomeObject = this; // No error
    var b : SomeObject = "3";  // Error
    var c : Int = this;        // Error
  }
}

class AnotherObject < SomeObject {
  someMethod() : Void {
    var a : AnotherObject = this; // No error
    var b : AnotherObject = "3";  // Error
    var c : Int = this;           // Error
    var d : SomeObject = this;    // No error
    return 3;
  }
}
