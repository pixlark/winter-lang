func return_record() {
	record Internal {
        field_a,
        field_b,
    }
    x = Internal("a", "b");
    return x;
}

x = return_record();
print(x);

x.field_a = 15;
print(x.field_a, x);
