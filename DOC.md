# Ideas and principles

1. lovela should be easy to use.
2. lovela should be accessible regardless of the user's cultural background.
3. lovela should be accessible and relevant regardless of the user's technical level.
4. lovela should look similar to a natural written language.
5. lovela shall yield safe and secure programs.
6. lovela should yield efficient and performant programs.
 
## Easy to use

This fundamental principle should discourage complicated and inaccessible features. 
It's still not easy to leverage considering the following principles.

## Accessible regardless of the user's cultural background

lovela should encourage ease of use regardless of the user's cultural background.

### Keywords are avoided

Language features that benefit from having readable names are implemented in the standard library. 
The standard library can then be translated to various languages.

### Code can be arranged in left to right or right to left order

If the language is 

### Other consequences

Other principles should probably also be considered, but that's beyond my limited knowledge of non-western cultures.

## Accessible and relevant regardless of the user's technical level

lovela should be accessible for beginners and people without rigorous knowledge of programming.

### Complicated syntax and technical constructs should be avoided

In my opinion, many programming languages suffer from an over-engineered syntax that establishes a hotbed for elitism and exclusion.

## Look similar to a natural written language

lovela should encourage a functional and fluent code style that is readable from start to end.

## Safe and secure programs

### No undefined behaviour

## Efficient and performant programs

# Common features

## Comments

```
<< this is a single line comment >>
```
```
<<
this is a 
multi line comment
>>
```
```
<< comments
<< can << be >>
nested
>>>>
```

## Strings

```
'strings are marked with single quotation marks'
```
```
'strings can be 
multi line'
```
```
'quotation marks '' are escaped by having two in a row'
```

## Integer types

### Syntax

`#N` for signed integer types where `N` is the number of bits. 1, 8, 16, 32 and 64 bits are supported.

`#0N` for unsigned integer types where `N` is the number of bits. 1, 8, 16, 32 and 64 bits are supported.

`#.N` for floating point types where `N` is the number of bits. 32 and 64 bits are supported.

For predefined numeric type names, see types in the standard library.

### Examples

`#32` is a common signed 32 bit integer.

`#08` is an 8 bit unsigned integer, a byte.

`#.64` is a 64 bit double precision floating point number.

# Functions

## Exmple

```
<< this is an imported function that prints a line of text >>
-> io/print_line.

<< this function takes its input and passes it on to io/print_line >>
print_message_1: io/print_line.

<< this function also explicitly states its input type >>
[typ/string] print_message_2: io/print_line.

<< this function contains multiple statements and thus needs parentheses for its definition >>
print_three_messages:
(
	<< the recommended way is to pass the input to a function in a fluent, left to right style (or right to left, if desired!) >>
	'First message' print_message_1.

	<< this function call works the same but here it's important that the input is a string >>
	'Second message' print_message_2.

	<< if the input is omitted it's possible to pass the input as the first parameter instead >> 
	print_message_1 ('Third message').
).

<< this is the anonymous main function that simply calls another function >>
: print_three_messages.
```

## Syntax

`import_export in_type name parameter_list out_type definition`

where

`import_export` is an optional import `->` or export `<-` specifier.

`in_type` is an optional input type (object type).

`name` is an optional function name. Can be omitted for the main function `: definition.` and for anonymous functions `[]() : definition.`.

`parameter_list` is an optional list of additional parameters `(param1 param1_type, ...)`.

`out_type` is an optional output type (return type).

`definition` is the optional function body `: << code goes here >> .`.

## Main function

### Syntax

`: definition.`

### Example

```
-> io/print_line. << import >>
: io/print_line 'Hello, World!'.
```

## Imported functions

## Exported functions

# Standard library

## Types

### Numeric types

#### Integer types

`typ/int8` is the same as `#8`.

`typ/int16` is the same as `#16`.

`typ/int32` is the same as `#32`.

`typ/int64` is the same as `#64`.

`typ/uint8` is the same as `#08`.

`typ/uint16` is the same as `#016`.

`typ/uint32` is the same as `#032`.

`typ/uint64` is the same as `#064`.

#### Boolean type

`typ/bool` is stored as `#1`. On initialization it will be set to 0 if the input is 0, or 1 otherwise.

#### Floating point types

`typ/float32` is the same as `#.32`.

`typ/float64` is the same as `#.64`.

### Casting

#### Implicit casts

Numberic types are implicitly cast if the source type fits in the target type.

##### Examples of implicit casts

`#32` and `#032` can both be cast to `#64` or `#064`.

`#32`and `#032` can both be cast to `#.64` but not to `#.32`.

`#032`cannot be cast to `#32` nor can `#32` be cast to `#032`.

#### Explicit casts

`typ/truncate` can be used to cast any numeric type to a narrower type. The value will be truncated if it doesn't fit in the target type.

`typ/cast` can be used to cast any numeric type to a narrower type. A runtime error will be raised if the value doesn't fit in the target type.

## I/O

`io/print_line` prints a line of text to the standard output stream.
`io/print` prints text to the standard output stream.

## Memory

`mem/get` allocates memory for an object of the given type and initializes it.

## String

# Built-in data structures

## Variable

Single variable type.

Syntax:
`type`

## Fixed array

Fixed-size (static) homogeneous (single-type) array.

Syntax:
`type#size`

C++ equivalent:
`std::array`

## Dynamic array

Dynamic (resizable), homogeneous (single-type) array.

Syntax:
`type#`

C++ equivalent:
`std::vector`

## Dynamic named array

Associative, dynamic (resizable), homogeneous (single-type) array. Keys are of type string.

Syntax:
`type#`

C++ equivalent:
`std::map`

## Fixed tuple

Fixed-size (static) heterogeneous (multi-type) tuple.

C++ equivalent:
`std::tuple`

## Named tuple

Associative, fixed-size (static) heterogeneous (multi-type) tuple.

C++ equivalent:
`std::tuple` with compile-time defined `std::array` of names.

## Dynamic tuple

Possibly TODO.

## Dynamic named tuple

Possibly TODO.

# Namespaces

`io/stream/out`
`io|stream|out`
