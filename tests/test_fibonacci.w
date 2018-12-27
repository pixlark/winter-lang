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
