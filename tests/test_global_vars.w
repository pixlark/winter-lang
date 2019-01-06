x = 10;

func a() {
	func b() {
        func c() {
            func d() {
                print(x);
            }
            d();
        }
        c();
    }
    b();
}

a();
