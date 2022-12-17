# For my memory

## Small

/type/bool

Bind operator
* Bind type
* Bind function

Built-in comparison, arithmetic, bitwise operators

Standard library namespaces
* mem
* io
* str
* re
* type
* net

## Medium

Assignment

Stream in/out syntax

Use msgpack for serialization

Select operator: object|property
* Index
* Name

Data structure length operator: object#

String interpolation

Function objects

Error handlers: [!]

Construction

Target language code blocks

Standard library: http?

## Large

Type system
* [1] -> flag. << new "type" called "flag", really a function "flag" that gives a default value of type [flag]. >>
* [flag] raise_error_if_set: << body >>. << function that takes input of type [flag] >>

Type construction
* [[100] checked] -> checked_integer_that_can_hold_only_-100_to_100 << Checked<100> >> 
* [[100] checked unsigned] -> checked_integer_that_can_hold_only_0_to_100 << Checked<100, 0> >> 

Built-in data structures
* Dynamic named array
* Dynamic tuple
* Dynamic named tuple

Standard library
