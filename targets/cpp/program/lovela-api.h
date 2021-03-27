#ifndef LOVELA_API
#define LOVELA_API

#ifndef LOVELA
#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif
typedef float float32_t;
typedef double float64_t;
#endif

#ifdef __cplusplus
extern "C" {
#include "lovela-exports.h"
}
#else
#include "lovela-exports.h"
#endif

#endif
