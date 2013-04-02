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

struct BusController;

void BusClearRCPInterrupt(struct BusController *, unsigned);
void BusRaiseRCPInterrupt(struct BusController *, unsigned);

#endif

