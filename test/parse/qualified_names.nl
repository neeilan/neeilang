// TODO: Because func vs var parsing on types,
// need to revisit after adding namespace-aware
// entity tracking
::qux::Baz x( foo::bar::T arg1 ) {
    a::b::C x;
    return;
}

qux::Baz y( ::foo::bar::T arg1 ) {
    return;
}