a = 0;
b = 1;

max = 10;
i = 0;

loop {
    i = i + 1;
    if i > max {
        break;
    }
    
	print a;
    
    temp = b;
    b = a + b;
    a = temp;
}
