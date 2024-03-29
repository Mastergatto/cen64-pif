/* ============================================================================
 *  Defs.h: Reality Signal Processor (RSP) Defines.
 *
 *  PIFSIM: Peripheral InterFace SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __PIF__DEFINITIONS_H__
#define __PIF__DEFINITIONS_H__
#include "Common.h"

/* MI_INTR_REG bits. */
#define MI_INTR_SI                0x02

/* Controller Buttons. */
#define BUTTON_A                  0x8000
#define BUTTON_B                  0x4000
#define BUTTON_Z                  0x2000
#define BUTTON_START              0x1000
#define BUTTON_JOY_UP             0x0800
#define BUTTON_JOY_DOWN           0x0400
#define BUTTON_JOY_LEFT           0x0200
#define BUTTON_JOY_RIGHT          0x0100
#define BUTTON_L                  0x0020
#define BUTTON_R                  0x0010
#define BUTTON_C_UP               0x0008
#define BUTTON_C_DOWN             0x0004
#define BUTTON_C_LEFT             0x0002
#define BUTTON_C_RIGHT            0x0001

#endif

