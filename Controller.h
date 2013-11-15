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

#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif

enum SIRegister {
#define Y(reg) reg,
#include "Registers.md"
#undef Y
  NUM_SI_REGISTERS
};

typedef enum {
    INVALID = -1,
    KEYBOARD = 0,
    MAYFLASH_N64 = 1,
    RETROLINK = 2,
    XBOX360 = 3,
    WIIU = 4,
} CONTROLTYPE;

#ifndef NDEBUG
extern const char *SIRegisterMnemonics[NUM_SI_REGISTERS];
#endif

struct BusController;

struct PIFController {
  struct BusController *bus;

  const uint8_t *rom;
  FILE *eepromFile;
  uint32_t regs[NUM_SI_REGISTERS];
  uint32_t status;

  uint8_t command[PIF_RAM_ADDRESS_LEN];
  uint8_t ram[PIF_RAM_ADDRESS_LEN];
  uint8_t eeprom[2048];
  CONTROLTYPE input;
};

struct PIFController *CreatePIF(const char *);
void DestroyPIF(struct PIFController *);
void SetEEPROMFilename(struct PIFController *, const char *);
void SetControlType(struct PIFController *, const char *);

#endif

