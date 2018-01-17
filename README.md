# Speckle Language

A very small, not fun to use programming language aimed at being simple to implement and nothing else.

Speckle compiles directly to x86 Assembly (GAS, AT&T syntax). It has a direct dependency on the GCC compiler, which it assumes to be in `/usr/bin/gcc`. 

Speckle was developed in C, tested on Arch Linux, with GCC version 7.2.1.

# Language Constructs

The language has a relatively small number (25) of symbols:

`fn`, `while`, `var`, `if`, `ret`, `{`, `}`, `(`, `)`, `+`, `-`, `/`, `*`, `%`, `<=`, `<`, `>`, `>=`, `=`, `==`, `;`, `&`, `|`, `!`, `'`.

Everything must be inside of a function. The main function is formed as follows:
~~~~
fn main(){
  ...
}
~~~~

where `...` are statements to be executed. Generally, a function named `foo` which takes two arguments, `a` and `b`, and simply prints `42` to the console, will look like:

~~~~
fn foo(a, b){
  printn(42);
}
~~~~

## Data Types

Speckle has just one true data type -- 64 bit numbers. Any variable created will be stored as an entire 64 bit word. Variable declarations look like:

~~~~
var name = 'G';
var name2;
name2 = 42;
~~~~

Speckle doesn't allow Strings directly (ie, there is no double quotes), but it does allow arrays of 64 bit values:

~~~~
var arr = {5};
arr{0} = 'H';
arr{1} = 'E';
arr{2} = 'L';
arr{3} = 'L';
arr{4} = 'O';
~~~~

Creating an array makes a call to the native malloc implementation on the system, and arr will now be set to the pointer returned. You could do arithmetic to this pointer, but it's not recommended. Obtaining the length of an array is done via the built-in function `len`, which takes the FIRST array position as an argument. Therein,

~~~~
var arr = {5};
var size = len(arr);
~~~~

is legal, but

~~~~
var arr = {5};
arr = arr + 4;
var size = len(arr);
~~~~

will not return expected results. This is because speckle stores the size of an array at the -1 index. Thus, overwriting this value will have adverse consequences. Incrementing the arr variable by 4 (moving 4 bytes off of the current position, effectively the location of `arr{1}`) will return whatever the value of `arr{0}` is.

## If Statements

Speckle doesn't allow else conditions. Instead, you are allowed to do if branches and then test if the negation of your branch occured:

~~~~
var cond = 1 & 0;
if(cond){
  printn(1);
};

if(!cond){
  printn(0);
}
~~~~

## Built-in functions

Speckle has a number of built in functions, with more to come. Namely:

* `printn(d)`: Prints the numerical value of `d` to the console.
* `printc(d)`: Prints the character value of `d` to the console.
* `newline()`: Prints `\n` to the console.
* `read()`: Attempts to read a byte from `stdin`, currently not fully functional.
* `malloc(d)`: Mallocs space for `d` 64-bit numbers, ie `d*4` bytes, and returns the start. Also will set the length of this segment.
* `len(d)`: Returns the length of the memory segment in terms of number of 64-bit numbers (ie: the same number as requested in malloc).

## Arithmetic and Logic

Speckle allows the common arithmetic operations: +, -, \*, /, and %. 

The following logical operators are allowed: & (and), | (or), <=, <, >, >=, ==, and ! (not).

## Expressions

Expressions can not have nesting. That is,

~~~~
var x = 1 & 2 | 3 & 4;
~~~~

is not allowed. Instead, one would have to specify it in pieces:

~~~~
var x = 1 & 2;
var y = 3 & 4;
var z = x | y;
~~~~

Similarly, the following isn't allowed:

~~~~
var x = 1 + 2 + 3;
~~~~

instead, one would have to do:

~~~~
var x = 1 + 2;
var y = x + 3;
~~~~
