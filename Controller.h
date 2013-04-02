/* ============================================================================
 *  Controller.h: PIF controller.
 *
 *  PIFSIM: Peripheral InterFace SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __PIF__CONTROLLER_H__
#define __PIF__CONTROLLER_H__
#include "Address.h"
#include "Common.h"

enum SIRegister {
#define Y(reg) reg,
#include "Registers.md"
#undef Y
  NUM_SI_REGISTERS
};

#ifndef NDEBUG
extern const char *SIRegisterMnemonics[NUM_SI_REGISTERS];
#endif

struct BusController;

struct PIFController {
  struct BusController *bus;

  const uint8_t *rom;
  uint32_t regs[NUM_SI_REGISTERS];
  uint8_t ram[PIF_RAM_ADDRESS_LEN];
};

struct PIFController *CreatePIF(const char *rompath);
void DestroyPIF(struct PIFController *);

#endif

