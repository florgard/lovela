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

#ifndef LOVELA
#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif
typedef float float32_t;
typedef double float64_t;
#endif

#include "lovela-exports.h"

#endif
