# Winter

## A dynamic, interpreted programming language

Winter is a very-early-in-development programming language designed to
support both iterative and functional principles.

### Some example code

```
func fibonacci(n) {
    a = 0;
    b = 1;
    fibs = [];

    loop {
        if list_count(fibs) > n {
            break;
        }

        list_append(fibs, a);
        
        temp = b;
        b = a + b;
        a = temp;
    }

    return fibs;
}

print(fibonacci(10));
```

produces

```
[0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55]
```
