#ifndef STDINT_H
#define STDINT_H

typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;

typedef signed char     int8_t;
typedef signed short    int16_t;
typedef signed int      int32_t;

// Some math
#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)
#define ABS(a)    (a < 0 ? -a : a)
#define CLAMP(min, max, a)  (MIN(max, MAX(min, a)))

#endif // STDINT_H

