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

void SIHandleDMARead(struct PIFController *);
void SIHandleDMAWrite(struct PIFController *);

int ReadEEPROMFile(struct PIFController *);
void SetEEPROMFile(struct PIFController *, const char *);
int WriteEEPROMFile(struct PIFController *);
void SetControlType(struct PIFController *, const char *);

#endif

