
fn main() : Int {
  
  for (var i = 0; i < 1000; i = i + 1) {
    // 966 crashes (segfault) - why??
    var new_arr : Int[ 965 ];
    new_arr[0] = 1;
  }
  
  print "OK";
  return 0;
}