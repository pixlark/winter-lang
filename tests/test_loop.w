i = 0;
loop {
    if i < 5 {
        i = i + 1;
    } else {
        break;
    }
    
    print("hello");
}

i = 0;
loop {
    i = i + 1;
    if i == 4 { continue; }
    print(i);
    if i == 10 { break; }
}
