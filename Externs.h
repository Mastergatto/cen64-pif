/* ============================================================================
 *  Externs.h: External definitions for the PIF plugin.
 *
 *  PIFSIM: Peripheral InterFace SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __PIF__EXTERNS_H__
#define __PIF__EXTERNS_H__
#include "Common.h"

struct BusController;

void BusClearRCPInterrupt(struct BusController *, unsigned);
void BusRaiseRCPInterrupt(struct BusController *, unsigned);

void DMAFromDRAM(struct BusController *, void *, uint32_t, uint32_t);
void DMAToDRAM(struct BusController *, uint32_t, const void *, size_t);

#endif

