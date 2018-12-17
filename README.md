# Winter

## A dynamic, interpreted programming language

Winter is a very-early-in-development programming language designed to
support both iterative and functional principles.

### Some example code

```
func my_function(arg) {
	if true {
		print arg + 1;
	} else {
		print 0;
	}
}

x = my_function;
x(1500);
```

produces

```
1501
```
