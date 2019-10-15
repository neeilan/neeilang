class SomeObject {
  someMethod() : Void {
    var a : SomeObject = this; // No error
    var b : SomeObject = "3";  // Error
    var c : Int = this;        // Error
  }
}

class AnotherObject {
  someMethod() : Voids{
    var a : AnotherObject = this; // No error
    var b : AnotherObject = "3";  // Error
    var c : Int = this;           // Error
  }
}
