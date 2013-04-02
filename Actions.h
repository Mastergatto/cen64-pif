/* ============================================================================
 *  Action.h: PIF controller functionality.
 *
 *  PIFSIM: Peripheral InterFace SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __PIF__ACTION_H__
#define __PIF__ACTION_H__
#include "Common.h"
#include "Controller.h"

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

void DMAFromDRAM(struct BusController *, void *, uint32_t, uint32_t);
void DMAToDRAM(struct BusController *, uint32_t, const void *, size_t);

void SIHandleDMARead(struct PIFController *);
void SIHandleDMAWrite(struct PIFController *);

#endif

