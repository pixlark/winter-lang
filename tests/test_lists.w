x = [];
list_append(x, 10);
list_append(x, "hello");
print(x, list_count(x));
x[1] = 15.3;
print(list_pop(x));
print(x, list_count(x));
