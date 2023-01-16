#include "esc_coe.h"
#include "utypes.h"
#include <stddef.h>

#ifndef HW_REV
#define HW_REV "1.0"
#endif

#ifndef SW_REV
#define SW_REV "1.0"
#endif

static const char acName1000[] = "Device Type";
static const char acName1008[] = "Device Name";
static const char acName1009[] = "Hardware Version";
static const char acName100A[] = "Software Version";
static const char acName1018[] = "Identity Object";
static const char acName1018_00[] = "Max SubIndex";
static const char acName1018_01[] = "Vendor ID";
static const char acName1018_02[] = "Product Code";
static const char acName1018_03[] = "Revision Number";
static const char acName1018_04[] = "Serial Number";
static const char acName1600[] = "LEDs";
static const char acName1600_00[] = "Max SubIndex";
static const char acName1600_01[] = "Connection_Start";
static const char acName1600_02[] = "Connection_Stop";
static const char acName1600_03[] = "Conveyor_Start";
static const char acName1600_04[] = "Conveyor_Stop";
static const char acName1600_05[] = "System_Start";
static const char acName1600_06[] = "System_Stop";
static const char acName1600_07[] = "Homing";

static const char acName1A00[] = "Positions";
static const char acName1A00_00[] = "Max SubIndex";
static const char acName1A00_01[] = "Position_X";
static const char acName1A00_02[] = "Position_Y";
static const char acName1A00_03[] = "Position_Z";
static const char acName1A00_04[] = "Position_R";
static const char acName1A00_05[] = "Connection_State";
static const char acName1A00_06[] = "Conveyor_State";
static const char acName1A00_07[] = "System_State";

static const char acName1C00[] = "Sync Manager Communication Type";
static const char acName1C00_00[] = "Max SubIndex";
static const char acName1C00_01[] = "Communications Type SM0";
static const char acName1C00_02[] = "Communications Type SM1";
static const char acName1C00_03[] = "Communications Type SM2";
static const char acName1C00_04[] = "Communications Type SM3";
static const char acName1C12[] = "Sync Manager 2 PDO Assignment";
static const char acName1C12_00[] = "Max SubIndex";
static const char acName1C12_01[] = "PDO Mapping";
static const char acName1C13[] = "Sync Manager 3 PDO Assignment";
static const char acName1C13_00[] = "Max SubIndex";
static const char acName1C13_01[] = "PDO Mapping";

static const char acName6000[] = "Positions";
static const char acName6000_00[] = "Max SubIndex";
static const char acName6000_01[] = "Position_X";
static const char acName6000_02[] = "Position_Y";
static const char acName6000_03[] = "Position_Z";
static const char acName6000_04[] = "Position_R";
static const char acName6000_05[] = "Connection_State";
static const char acName6000_06[] = "Conveyor_State";
static const char acName6000_07[] = "System_State";

static const char acName7000[] = "LEDs";
static const char acName7000_00[] = "Max SubIndex";
static const char acName7000_01[] = "Connection_Start";
static const char acName7000_02[] = "Connection_Stop";
static const char acName7000_03[] = "Conveyor_Start";
static const char acName7000_04[] = "Conveyor_Stop";
static const char acName7000_05[] = "System_Start";
static const char acName7000_06[] = "System_Stop";
static const char acName7000_07[] = "Homing";
static const char acName8000[] = "Parameters";
static const char acName8000_00[] = "Max SubIndex";
static const char acName8000_01[] = "Multiplier";

const _objd SDO1000[] =
{
  {0x0, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1000, 0x01901389, NULL},
};
const _objd SDO1008[] =
{
  {0x0, DTYPE_VISIBLE_STRING, 88, ATYPE_RO, acName1008, 0, "evb9252_dig"},
};
const _objd SDO1009[] =
{
  {0x0, DTYPE_VISIBLE_STRING, 0, ATYPE_RO, acName1009, 0, HW_REV},
};
const _objd SDO100A[] =
{
  {0x0, DTYPE_VISIBLE_STRING, 0, ATYPE_RO, acName100A, 0, SW_REV},
};
const _objd SDO1018[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1018_00, 4, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_01, 0x1337, NULL},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_02, 1234, NULL},
  {0x03, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_03, 0, NULL},
  {0x04, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_04, 0x00000000, &Obj.serial},
};
const _objd SDO1600[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_00, 7, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1600_01, 0x70000101, NULL},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1600_02, 0x70000201, NULL},
  {0x03, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1600_03, 0x70000301, NULL},
  {0x04, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1600_04, 0x70000401, NULL},
  {0x05, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1600_05, 0x70000501, NULL},
  {0x06, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1600_06, 0x70000601, NULL},
  {0x07, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1600_07, 0x70000701, NULL},
};
const _objd SDO1A00[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_00, 7, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1A00_01, 0x60000120, NULL},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1A00_02, 0x60000220, NULL},
  {0x03, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1A00_03, 0x60000320, NULL},
  {0x04, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1A00_04, 0x60000420, NULL},
  {0x05, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1A00_05, 0x60000501, NULL},
  {0x06, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1A00_06, 0x60000601, NULL},
  {0x07, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1A00_07, 0x60000701, NULL},
};
const _objd SDO1C00[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C00_00, 4, NULL},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C00_01, 1, NULL},
  {0x02, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C00_02, 2, NULL},
  {0x03, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C00_03, 3, NULL},
  {0x04, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C00_04, 4, NULL},
};
const _objd SDO1C12[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C12_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C12_01, 0x1600, NULL},
};
const _objd SDO1C13[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C13_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C13_01, 0x1A00, NULL},
};
const _objd SDO6000[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName6000_00, 7, NULL},
  {0x01, DTYPE_REAL32, 32, ATYPE_RO | ATYPE_TXPDO, acName6000_01, 0, &Obj.Positions.Position_X},
  {0x02, DTYPE_REAL32, 32, ATYPE_RO | ATYPE_TXPDO, acName6000_02, 0, &Obj.Positions.Position_Y},
  {0x03, DTYPE_REAL32, 32, ATYPE_RO | ATYPE_TXPDO, acName6000_03, 0, &Obj.Positions.Position_Z},
  {0x04, DTYPE_REAL32, 32, ATYPE_RO | ATYPE_TXPDO, acName6000_04, 0, &Obj.Positions.Position_R},
  {0x05, DTYPE_BOOLEAN, 1, ATYPE_RO | ATYPE_TXPDO, acName6000_05, 0, &Obj.States.Connection},
  {0x06, DTYPE_BOOLEAN, 1, ATYPE_RO | ATYPE_TXPDO, acName6000_06, 0, &Obj.States.Conveyor},
  {0x07, DTYPE_BOOLEAN, 1, ATYPE_RO | ATYPE_TXPDO, acName6000_07, 0, &Obj.States.System},
};
const _objd SDO7000[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName7000_00, 7, NULL},
  {0x01, DTYPE_BOOLEAN, 1, ATYPE_RO | ATYPE_RXPDO, acName7000_01, 0, &Obj.Buttons.Connection_Start},
  {0x02, DTYPE_BOOLEAN, 1, ATYPE_RO | ATYPE_RXPDO, acName7000_02, 0, &Obj.Buttons.Connection_Stop},
  {0x03, DTYPE_BOOLEAN, 1, ATYPE_RO | ATYPE_RXPDO, acName7000_03, 0, &Obj.Buttons.Conveyor_Start},
  {0x04, DTYPE_BOOLEAN, 1, ATYPE_RO | ATYPE_RXPDO, acName7000_04, 0, &Obj.Buttons.Conveyor_Stop},
  {0x05, DTYPE_BOOLEAN, 1, ATYPE_RO | ATYPE_RXPDO, acName7000_05, 0, &Obj.Buttons.System_Start},
  {0x06, DTYPE_BOOLEAN, 1, ATYPE_RO | ATYPE_RXPDO, acName7000_06, 0, &Obj.Buttons.System_Stop},
  {0x07, DTYPE_BOOLEAN, 1, ATYPE_RO | ATYPE_RXPDO, acName7000_07, 0, &Obj.Buttons.Homing},
};
const _objd SDO8000[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName8000_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RW, acName8000_01, 0, &Obj.Parameters.Multiplier},
};

const _objectlist SDOobjects[] =
{
  {0x1000, OTYPE_VAR, 0, 0, acName1000, SDO1000},
  {0x1008, OTYPE_VAR, 0, 0, acName1008, SDO1008},
  {0x1009, OTYPE_VAR, 0, 0, acName1009, SDO1009},
  {0x100A, OTYPE_VAR, 0, 0, acName100A, SDO100A},
  {0x1018, OTYPE_RECORD, 4, 0, acName1018, SDO1018},
  {0x1600, OTYPE_RECORD, 7, 0, acName1600, SDO1600},
  {0x1A00, OTYPE_RECORD, 7, 0, acName1A00, SDO1A00},
  {0x1C00, OTYPE_ARRAY, 4, 0, acName1C00, SDO1C00},
  {0x1C12, OTYPE_ARRAY, 1, 0, acName1C12, SDO1C12},
  {0x1C13, OTYPE_ARRAY, 1, 0, acName1C13, SDO1C13},
  {0x6000, OTYPE_RECORD, 7, 0, acName6000, SDO6000},
  {0x7000, OTYPE_RECORD, 7, 0, acName7000, SDO7000},
  {0x8000, OTYPE_RECORD, 1, 0, acName8000, SDO8000},
  {0xffff, 0xff, 0xff, 0xff, NULL, NULL}
};
