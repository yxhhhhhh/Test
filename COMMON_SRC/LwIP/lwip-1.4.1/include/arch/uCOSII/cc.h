#ifndef __CC_H__
#define __CC_H__

#include "cpu.h"

typedef unsigned char          u8_t;;
typedef signed char             s8_t;
typedef unsigned short         u16_t;    
typedef signed short            s16_t;
typedef unsigned long          u32_t;
typedef signed long             s32_t;    
typedef u32_t                   mem_ptr_t;

#define U16_F   "hu"
#define S16_F   "hd"
#define X16_F   "hx"
#define U32_F   "lu"
#define S32_F   "ld"
#define X32_F   "lx"

#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT __attribute__ ((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x)    x

#ifndef LWIP_PLATFORM_ASSERT
#define LWIP_PLATFORM_ASSERT(x) \
    do \
    {   printf("Assertion \"%s\" failed at line %d in %s\n", x, __LINE__, __FILE__); \
    } while(0)
#endif

#ifndef LWIP_PLATFORM_DIAG
#define LWIP_PLATFORM_DIAG(x) do {printf x;} while(0)
#endif

#endif
