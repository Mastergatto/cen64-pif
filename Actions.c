/* ============================================================================
 *  Action.c: PIF controller functionality.
 *
 *  PIFSIM: Peripheral InterFace SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Actions.h"
#include "Common.h"
#include "Controller.h"
#include "Definitions.h"
#include "Externs.h"

#ifdef __cplusplus
#include <cassert>
#include <cstring>
#else
#include <assert.h>
#include <string.h>
#endif

/* ============================================================================
 *  SIHandleDMARead: Invoked when SI_PIF_ADDR_RD64B_REG is written.
 *
 *  PIF RAM = Source.
 *  SI_DRAM_ADDR_REG = DRAM (target) address.
 *  64 bytes = Transfer size.
 * ========================================================================= */
void SIHandleDMARead(struct PIFController *controller) {
  uint32_t target = controller->regs[SI_DRAM_ADDR_REG] & 0x1FFFFFFF;
  assert(((target & 0x3) == 0) && "Unaligned access.");

  /* Should process now... */
  /*controller->ram[0x3F] = 0;*/

  debug("DMA | Request: Read from PIF RAM.");
  debugarg("DMA | DEST   : [0x%.8x].", target);
  debugarg("DMA | SOURCE : [0x%.8x].", PIF_RAM_BASE_ADDRESS);
  debugarg("DMA | LENGTH : [0x%.8x].", 64);

  DMAToDRAM(controller->bus, target, controller->ram, 64);

  controller->regs[SI_STATUS_REG] |= 0x1000;
  BusRaiseRCPInterrupt(controller->bus, MI_INTR_SI);
}

/* ============================================================================
 *  SIHandleDMAWrite: Invoked when SI_PIF_ADDR_WR64B_REG is written.
 *
 *  PIF RAM = Target.
 *  SI_DRAM_ADDR_REG = DRAM (source) address.
 *  64 bytes = Transfer size.
 * ========================================================================= */
void SIHandleDMAWrite(struct PIFController *controller) {
  uint32_t source = controller->regs[SI_DRAM_ADDR_REG] & 0x1FFFFFFF;
  assert(((source & 0x3) == 0) && "Unaligned access.");

  debug("DMA | Request: Write to PIF RAM.");
  debugarg("DMA | DEST   : [0x%.8x].", PIF_RAM_BASE_ADDRESS);
  debugarg("DMA | SOURCE : [0x%.8x].", source);
  debugarg("DMA | LENGTH : [0x%.8x].", 64);

  DMAFromDRAM(controller->bus, controller->ram, source, 64);

  controller->regs[SI_STATUS_REG] |= 0x1000;
  BusRaiseRCPInterrupt(controller->bus, MI_INTR_SI);
}

