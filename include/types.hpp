#ifndef AD_TYPES_H
#define AD_TYPES_H

#include <cstdint>

typedef int8_t   int8;
typedef int16_t  int16;
typedef int32_t  int32;
typedef int64_t  int64;
typedef uint8_t  uint8;
typedef uint8_t  uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef float    float32;
typedef double   float64;

#define AD_PATH_SIZE 256

#define ad_return_t int32
#define AD_RETURN_SUCCESS 0
#define AD_RETURN_NOT_DONE 0
#define AD_RETURN_BAD_FILE 1
#define AD_RETURN_BAD_HEADER 2
#define AD_RETURN_DONE 3

#endif
