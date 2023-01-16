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
      uint8_t System;
      uint8_t Homing;
      uint8_t Blue_Local;
   } States;

   struct
   {
      float Position_X;
      float Position_Y;
      float Position_Z;
      float Position_R;
   } Positions;

   /* Outputs */

   struct
   {
      uint8_t System_Start;
      uint8_t System_Stop;
      uint8_t Homing;
   } Buttons;

   struct
   {
      uint8_t Red;
      uint8_t Green;
      uint8_t Blue;
      uint8_t Sequence_Number;
   } Counters;

   /* Parameters */

   struct
   {
      uint32_t Multiplier;
   } Parameters;

} _Objects;

extern _Objects Obj;

#endif /* __UTYPES_H__ */
