# Simple Calculator
This piece of code was an assignment for my datastructures course. The calculator parses an
expression in polish notation and has the ability to simplify, or differentiate on x; both in a
limited fashion. The parsed expression may be printed in inflix notation or may be written to a DOT file.

I use a nice technique to make the tree cachable. The program allocates an arena of trees, ie a page of trees in the memory.
Using the ZII idiom we may retrieve individual trees from this arena without having to validate each pointer, see tree.c if
you are interested. Otherwise the simplify logic is quite simpel and only simplifies trees locally. Differentiation is only
applied on the variable x, therefore also not making things too complex. 

# Run

``` bash
$ ./boom.bin
# Simple calculator by Jenny Vermeltfoort.
# Run with flag '-s' to make it silent.
# exp <expression>      ; loads the expression, expression must be in polish notation.
# print                 ; print the loaded expression in infix notation.
# simp                  ; simplify the loaded expression.
# eval <value>          ; substitute x within the loaded expression for <value>, value may be point seperated.
# dot <filename>        ; write the loaded expression to a DOT file format.
# diff                  ; differentiates the loaded expression on x.
# end                   ; end the program.
# help                  ; print help.
$ 
```

# Compile
The code uses some GNU extended features of c, for example designated initializers, therefore may only be build using gcc.

``` bash
$ make boom.bin
```