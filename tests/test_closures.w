func outer(x) {
	func inner() {
        x = x + 1;
	    return x;
    }
    return inner;
}

x = outer(10);

i = 0;
loop {
    if i >= 5 { break; }
    i = i + 1;
    print(x());
}
