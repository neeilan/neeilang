var x : String = "hi";

{
  var x : Int = 3;
  x = 4;
  {
    x = 5;
  }

  x = "str";
}

x = "bye";
{
  x = 4;
}

