func outer(a) {
    func inner_1(x) {
        print x + 1;
    }
    func inner_2(x) {
        print x - 1;
    }
    if a > 0 {
        return inner_1;
    } else {
        return inner_2;
    }
}

f = outer(1);
f(1);

func triangle(x) {
	 if x == 1 {
	     return 1;
     } else {
         return x + triangle(x - 1);
     }
}

print triangle(5);
