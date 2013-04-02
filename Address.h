/* ============================================================================
 *  Address.h: Device address list.
 *
 *  PIFSIM: Peripheral InterFace SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __PIF__ADDRESS_H__
#define __PIF__ADDRESS_H__

/* PIF RAM */
#define PIF_RAM_BASE_ADDRESS      0x1FC007C0
#define PIF_RAM_ADDRESS_LEN       0x00000040

/* PIF RAM Offsets. */
#define CIC_STATUS_BYTE           0x3C

/* PIF ROM */
#define PIF_ROM_BASE_ADDRESS      0x1FC00000
#define PIF_ROM_ADDRESS_LEN       0x000007C0

/* Serial Interface Registers. */
#define SI_REGS_BASE_ADDRESS      0x04800000
#define SI_REGS_ADDRESS_LEN       0x0000001C

#endif

