# Naming Convention

Structs :: `Snake_Pascal_Case`.

Functions :: `snake_case`.

`#define`s :: `UPPER_SNAKE_CASE`.

Variables :: `snake_case`.

# Section Comments

Sections are delimited by a starting comment `// : $section_name` and an ending comment `// :\ $section_name`. Description of the sections takes place after the starting comment.

```
// : My_Struct

// This struct does some things

typedef struct {} My_Struct;

// :\ My_Struct
```

# Struct Methods

Functions that operate very closely with one specific struct follow a naming convention:

Does someting to `My_Struct` :: `void my_struct_do_something(My_Struct * my_struct, ...);`

Returns value of new `My_Struct` :: `My_Struct my_struct_new();`

Returns allocated pointer to new `My_Struct` :: `My_Struct my_struct_alloc();`
