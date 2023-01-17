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

### Passing variables

1. Pass by reference.
 * [out] == [in] -> return by reference
 * [out] != [in] -> return by value
2. Optimize by passing small types by value.
 * size <= 16 -> pass by value
  * template<typename T> concept bool SmallType = sizeof(T) <= 16;
 * size > 16 -> pass by reference
  * What about multiple arguments? Is 2^c overloads required?
 * [out] == [in] -> return by value/reference
 * [out] != [in] -> return by value

/mem/clone?
* std::move

### Other

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

### Analyzer phase

Checks that expressions and function calls are valid, etc.

* Check that functions exist.
* Check that input and output types are compatible.

### Type system
* [1] -> flag. << new "type" called "flag", really a function "flag" that gives a default value of type [flag]. >>
* [flag] raise_error_if_set: << body >>. << function that takes input of type [flag] >>

### Type construction
* [[100] checked] -> checked_integer_that_can_hold_only_-100_to_100 << Checked<100> >> 
* [[100] checked unsigned] -> checked_integer_that_can_hold_only_0_to_100 << Checked<100, 0> >> 

### Built-in data structures
* Dynamic named array
* Dynamic tuple
* Dynamic named tuple

### Standard library
