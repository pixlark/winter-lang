# Winter

## A dynamic, interpreted programming language

Winter is a very-early-in-development programming language designed to
support both iterative and functional principles.

### Some example code

```
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
```

produces

```
0
1
1
2
3
5
8
13
21
34
```
