#ifndef LOVELA_API
#define LOVELA_API

#ifdef __cplusplus
#define LOVELA_API_C extern "C"
#else
#define LOVELA_API_C extern
#endif

#define LOVELA_API_CPP extern

#ifdef LOVELA
// The lovela program/library is being built and exported.
#if defined(_MSC_VER)
#define LOVELA_API_DYNAMIC_EXPORT __declspec(dllexport)
#define LOVELA_API_DYNAMIC_IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
#define LOVELA_API_DYNAMIC_EXPORT __attribute__((visibility("default")))
#define LOVELA_API_DYNAMIC_IMPORT
#else
#define LOVELA_API_DYNAMIC_EXPORT
#define LOVELA_API_DYNAMIC_IMPORT
#endif
#else
// The interface to the lovela program/library is being imported.
#if defined(_MSC_VER)
#define LOVELA_API_DYNAMIC_EXPORT __declspec(dllimport)
#define LOVELA_API_DYNAMIC_IMPORT __declspec(dllexport)
#else
#define LOVELA_API_DYNAMIC_EXPORT
#define LOVELA_API_DYNAMIC_IMPORT
#endif
#endif

typedef bool l_i1;
typedef char l_i8;
typedef short l_i16;
typedef int l_i32;
typedef long long l_i64;
typedef bool l_u1;
typedef unsigned char l_u8;
typedef unsigned short l_u16;
typedef unsigned int l_u32;
typedef unsigned long long l_u64;
typedef float l_f32;
typedef double l_f64;
typedef const char* l_cstr;

// Edit user-imports.h to include headers or declare functions that the lovela program needs for its imported functions without explicit API specifications.
#include "user-imports.h"
// Automatically generated header with function declarations that the lovela program imports.
#include "lovela-imports.h"
// Automatically generated header with function declarations that the lovela program exports.
#include "lovela-exports.h"

#endif
