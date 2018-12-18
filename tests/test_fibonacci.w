func fibonacci(n) {
	a = 0;
	b = 1;
	
	i = 0;
	loop {
		i = i + 1;
		if i > n {
			break;
		}
		
		print a;
		
		temp = b;
		b = a + b;
		a = temp;
	}
}

fibonacci(10);
