#ifndef __UTYPES_H__
#define __UTYPES_H__

#include "cc.h"

/* Object dictionary storage */

typedef struct
{
   /* Identity */

   uint32_t serial;

   /* Inputs */

   struct
   {
      uint8_t Red;
      uint8_t Green;
      uint8_t Blue;
   } Colors;

   /* Outputs */

   /* Parameters */

   struct
   {
      uint32_t Multiplier;
   } Parameters;

} _Objects;

extern _Objects Obj;

#endif /* __UTYPES_H__ */
