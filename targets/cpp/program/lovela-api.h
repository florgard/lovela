#ifndef LOVELA_API
#define LOVELA_API

#ifdef __cplusplus
#define LOVELA_EXTERNAL_LINKAGE extern "C"
#endif

#define LOVELA_EXPORT LOVELA_EXTERNAL_LINKAGE

#ifdef _WIN32
// Naive assumption that all imports follow the active VC runtime (CRT) linkage
#define LOVELA_IMPORT LOVELA_EXTERNAL_LINKAGE _CRTIMP
#else
#define LOVELA_IMPORT LOVELA_EXTERNAL_LINKAGE
#endif

typedef char l_i8;
typedef short l_i16;
typedef int l_i32;
typedef long long l_i64;
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
