# TI-BASIC
An interpreter for a simplified version of the TI-BASIC language found on
TI-83/84 graphing calculators.  Some operators are modified so that they may be
more easily written with ASCII characters.

## How to Build
```
$ make
```
This will create the interpreter `tibasic` in the project root directory, and
all the object files will be in the `build` directory.  To clean up, just run
`make clean`.

## How to Install
```
$ sudo ./install.sh
```
This will run `make` then move the compiled binary to `/usr/local/bin`, and
finally call `make clean`.  If there is a compilation error, nothing should be
copied to that directory.

## How to Run
With the interpreter:
```
$ tibasic source_file
```
- `source_file`: Source file name

Without the interpreter: Include the [shebang](#shebangs) in the `source_file`.

## Implementation
The language currently only supports a limited subset of the original TI-BASIC
language.  This includes the following [list of commands](#supported-commands),
the following [list of operators](#supported-operators), `A`-`Z` and `Ans` real
variables, `not` function, and real numbers.

### Features
This implementation supports single line comments that begin with a `#`.  Note
that the line delimiter `':'` will not end the comment, only a newline character
`'\n'` will.  A consequence of this is that shebangs can be supported.  The
following is an example of a comment.
```
#This is a comment
```

#### Shebangs
A shebang is a special comment on the first line of a program that tells the shell
what interpreter to use to run the program.  It looks like the following:
```
#!/usr/bin/env tibasic
```
The first part `#` denotes a comment, but paired with a `!` denotes a shebang.
The following is a full path to the interpreter with optional arguments to be
supplied.  What this is really doing is calling `/usr/bin/env` with `tibasic` as
an argument.  That in turn looks for `tibasic` in the `$PATH` and runs that.

The benefit of the shebang is that it lets you execute the program without you
needing to specify the interpreter.  You can simply execute the file like so:
```
$ ./source_file
```
Note however that you must first have the execution permission for the file set.
You can do that like so:
```
$ chmod +x source_file
```
However this method of execution only works if you use the shebang, and the
`tibasic` interpreter can be found on the `$PATH`.

### Supported Commands
- `If `
- `Then`
- `Else`
- `For`
- `While `
- `Repeat `
- `End`
- `Prompt`
- `Disp `
- `ClrHome`

### Supported Operators
- `:`  : Line delimiter
- `->` : Assignment
- `()` : Parentheses
- `and`: And
- `or` : Or
- `xor`: Xor
- `=`  : Equal
- `!=` : Not equal
- `>`  : Greater than
- `>=` : Greater than or equal
- `<`  : Less than
- `<=` : Less than or equal
- `+`  : Addition
- `-`  : Subtraction
- `*`  : Multiplication
- `/`  : Division
- `~`  : Negation
- `^`  : Exponentiation
