# For my memory

## Small

Simpler basic types
* [100] instead of #8
* [200] instead of #+8
* [1000000] instead of #32
* What about floats? Exponential notation?
  * [+\-]?\d+\.\d+[+\-]?(e|E)\d+
  * [1.0e30] instead of #.32, [1.0e300] instead of #.64
* What about tagged types? [1] -> [#1]? [#key]
Identifiers beginning with number or operator
* 6.28 -> 2pi
* 50 -> %approved
* -20 -> -4_fahrenheit
* What about floats? Exponential notation?
  * OK: Dot (separator) not accepted in identifier.
Operators require whitespace
* max-min << identifier >>
* max - min << difference >>
What is an operator?
* +-*/%
What is a separator?
* .:,;!?|()[]<>
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
* [100] -> unchecked_integer_that_can_hold_at_least_-100_to_100 << int8 >>
* [100] unsigned -> unchecked_integer_that_can_hold_at_least_0_to_100 << uint8 >>
* [[100] unsigned checked] -> checked_integer_that_can_hold_only_-100_to_100 << Checked<uint8, 100> >> 
Built-in data structures
* Dynamic named array
* Dynamic tuple
* Dynamic named tuple
Standard library
