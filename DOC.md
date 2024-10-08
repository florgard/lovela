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

### Literals

`0`

`123`

`+123`

`-123`

### Standard types

`/type/i8`

`/type/u8`

`/type/i16`

`/type/u16`

`/type/i32`

`/type/u32`

`/type/i64`

`/type/u64`

### Type definition

`[n]` where `n` is a literal integer number that needs to fit in the data type.

The most narrow supported integer type that fits the literal number will be used. Signed types will be used before unsigned types.

Supported types are signed and unsigned 8, 16, 32, and 64 bit wide integer types.

Examples:

`[0]` yields a signed 8 bit integer type (for which the max value is 127). Same as `/type/i8`.

`[100]` yields a signed 8 bit integer type (for which the max value is 127). Same as `/type/i8`.

`[-100]` yields a signed 8 bit integer type (for which the min value is -128). Same as `/type/i8`.

`[200]` yields an unsigned 8 bit integer type (a byte, for which the max value is 255). Same as `/type/u8`.

`[100000]` yields a signed 32 bit integer type (for which the max value is 2147483647). Same as `/type/i32`.

`[10000000000000000000]` yields an unsigned 64 bit integer type (for which the max value is 18446744073709551615). Same as `/type/u64`.

## Floating point (real) types

### Literals

`0.0`

`123.45`

`+123.45`

`-123.45`

`1.0e6`

`1.0e+6`

`1.0e-6`

### Standard types

`/type/f32`

`/type/f64`

### Type definition

`[n]` where `n` is a literal real number that needs to fit in the data type.

The most narrow supported floating point type that fits the literal number will be used.

Supported types are 32 and 64 bit wide floating point types.

Examples:

`[0.0]` yields a 32 bit floating point type (for which the max value is in the order 1.0e38). Same as `/type/f32`.

`[1.0e30]` yields a 32 bit floating point type (for which the max value is in the order 1.0e38). Same as `/type/f32`.

`[1.0e300]` yields a 64 bit floating point (for which the min value is in the order 1.0e308). Same as `/type/f64`.

# Functions

## Exmple

```
<< this is an imported function that prints a line of text >>
-> /io/print_line.

<< this function takes its input and passes it on to /io/print_line >>
print_message_1: /io/print_line.

<< this function also explicitly states its input type >>
[/type/string] print_message_2: /io/print_line.

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
-> /io/print_line. << import >>
: /io/print_line 'Hello, World!'.
```

## Imported functions

## Exported functions

# Standard library

Standard library types and functions reside in the global namespace `/`.

The following libraries constitute the standard library:

`type` contains built-in data types and type related functions.

`str` contains string data types and functions.

`io` contains basic input and output functions for things like printing text, files, streams, system state, etc.

`mem` contains memory related functions, such as memory allocation.

`net` contains networking types and functions.

`re` contains regular expression types and functions.

## Types

### Numeric types

#### Integer types

`/type/i8` is a signed 8 bit integer type.

`/type/i16` is a signed 16 bit integer type.

`/type/i32` is a signed 32 bit integer type.

`/type/i64` is a signed 64 bit integer type.

`/type/u8` is an unsigned 8 bit integer type.

`/type/u16` is an unsigned 16 bit integer type.

`/type/u32` is an unsigned 32 bit integer type.

`/type/u64` is an unsigned 64 bit integer type.

#### Floating point (real) types

`/type/f32` is a 32 bit floating point type.

`/type/f64` is a 64 bit floating point type.

#### Boolean type

`/type/bool` is stored as `/type/u8`. On initialization it will be set to 0 if the input is 0, or 1 otherwise.

### Casting

#### Implicit casts

Numberic types are implicitly cast if the source type fits in the target type.

##### Rules for allowed implicit casts

`/type/bool` -> `/type/u8`

`/type/bool` -> `/type/i8`


`/type/u8` -> `/type/u16`

`/type/u8` -> `/type/i16`

`/type/i8` -> `/type/i16`


`/type/u16` -> `/type/u32`

`/type/u16` -> `/type/i32`

`/type/u16` -> `/type/f32`

`/type/i16` -> `/type/i32`

`/type/i16` -> `/type/f32`


`/type/u32` -> `/type/u64`

`/type/u32` -> `/type/i64`

`/type/u32` -> `/type/f64`

`/type/i32` -> `/type/i64`

`/type/i32` -> `/type/f64`

#### Explicit casts

`/type/truncate` can be used to cast any numeric type to a narrower type. The value will be truncated if it doesn't fit in the target type.

`/type/cast` can be used to cast any numeric type to a narrower type. A runtime error will be raised if the value doesn't fit in the target type.

## I/O

`/io/print_line` prints a line of text to the standard output stream.

`/io/print` prints text to the standard output stream.

## Memory

`/mem/get` allocates memory for an object of the given type and initializes it.

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

Namespaces are hierarchical and are expressed with a slash separated path much like a web address.

A user namespace doesn't begin with a slash since they are in the context of the code module.

`utility/math/`

Standard library namespaces begin with a slash since they are in the root context.

`/io/stream/`

When a namespace is specified it must end with a slash.

`/type/bool/`

When a function or type within a namespace is specified then there's no slash at the end.

`/type/bool`

`/io/stream/out`

A function or type can have the same name as a namespace in the same context.

`[/type/bool] << bool type >>`

`/type/bool << bool type initializer >>`

`/type/bool/ << bool namespace >>`

`/type/bool/is_true << bool namespace function >>`

# Dependencies

{fmt} - https://github.com/fmtlib/fmt
UTF8-CPP - https://github.com/nemtrif/utfcpp
generator - https://github.com/TartanLlama/generator
Magic Enum C++ - https://github.com/Neargye/magic_enum
