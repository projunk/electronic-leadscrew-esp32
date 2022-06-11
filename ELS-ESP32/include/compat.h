#ifndef COMPAT_H
#define COMPAT_H


#include <stdint.h>
#include <stdlib.h>

#define int16       int16_t
#define int32       int32_t
#define int64       int64_t
#define Uint16      uint16_t
#define Uint32      uint32_t
#define Uint64      uint64_t

#define IS_FLOAT_NULL(prmFloatValue) (abs(prmFloatValue) < 0.00001f)


#endif