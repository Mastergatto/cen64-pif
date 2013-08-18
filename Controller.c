/* ============================================================================
 *  Controller.c: PIF controller.
 *
 *  PIFSIM: Peripheral InterFace SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Address.h"
#include "Actions.h"
#include "Common.h"
#include "Controller.h"
#include "Definitions.h"
#include "Externs.h"

#ifdef __cplusplus
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#else
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

static void InitPIF(struct PIFController *, const uint8_t *);

/* ============================================================================
 *  Mnemonics table.
 * ========================================================================= */
#ifndef NDEBUG
const char *SIRegisterMnemonics[NUM_SI_REGISTERS] = {
#define Y(reg) #reg,
#include "Registers.md"
#undef Y
};
#endif

/* ============================================================================
 *  ConnectPIFToBus: Connects a PIF instance to a Bus instance.
 * ========================================================================= */
void
ConnectPIFToBus(struct PIFController *controller, struct BusController *bus) {
  controller->bus = bus;
}

/* ============================================================================
 *  CreatePIF: Creates and initializes an PIF instance.
 * ========================================================================= */
struct PIFController *
CreatePIF(const char *romPath) {
  struct PIFController *controller;
  uint8_t *romImage;
  size_t allocSize;
  FILE *romFile;
  long romSize;

  /* Open image, determine image size. */
  if ((romFile = fopen(romPath, "r")) == NULL) {
    debug("Failed to open PIFROM image.");

    return NULL;
  }

  if (fseek(romFile, 0, SEEK_END) == -1 || (romSize = ftell(romFile)) == -1) {
    debug("Failed to determine PIFROM size.");

    fclose(romFile);
    return NULL;
  }

  rewind(romFile);

  /* Allocate memory for controller and image. */
  allocSize = sizeof(struct PIFController) + romSize;
  if ((controller = (struct PIFController*) malloc(allocSize)) == NULL) {
    debug("Failed to allocate memory for PIFROM image.");

    fclose(romFile);
    return NULL;
  }

  romImage = (uint8_t*) controller + sizeof(*controller);
  InitPIF(controller, romImage);

  /* Read image into controller's memory. */
  if (fread(romImage, romSize, 1, romFile) != 1) {
    debug("Failed to read PIFROM image.");

    free(controller);
    controller = NULL;
  }

  fclose(romFile);
  return controller;
}

/* ============================================================================
 *  DestroyPIF: Releases any resources allocated for an PIF instance.
 * ========================================================================= */
void
DestroyPIF(struct PIFController *controller) {
  free(controller);
}

/* ============================================================================
 *  InitPIF: Initializes the PIF controller.
 * ========================================================================= */
static void
InitPIF(struct PIFController *controller, const uint8_t *romImage) {
  debug("Initializing PIF.");
  memset(controller, 0, sizeof(*controller));

  controller->rom = romImage;
}

/* ============================================================================
 *  PIFRAMReadByte: Read byte from PIF.
 * ========================================================================= */
int
PIFRAMReadByte(void *_controller, uint32_t address, void *_data) {
	struct PIFController *controller = (struct PIFController*) _controller;
	uint8_t *data = (uint8_t*) _data, byte;

  debugarg("PIFRAMReadByte: Read from address [0x%.8X]", address);
  address = address - PIF_RAM_BASE_ADDRESS;

  if (address == 0x24)
    controller->status = 0x80;

  else if (address == 0x3C) {
    *data = controller->status;
    return 0;
  }

  memcpy(&byte, controller->ram + address, sizeof(byte));
  *data = byte;

  return 0;
}

/* ============================================================================
 *  PIFRAMReadHWord: Read halfword from PIF.
 * ========================================================================= */
int
PIFRAMReadHWord(void *_controller, uint32_t address, void *_data) {
	struct PIFController *controller = (struct PIFController*) _controller;
	uint16_t *data = (uint16_t*) _data, hword;

  debugarg("PIFRAMReadHWord: Read from address [0x%.8X]", address);
  address = address - PIF_RAM_BASE_ADDRESS;

  if (address == 0x24)
    controller->status = 0x80;

  else if (address == 0x3C) {
    *data = controller->status;
    return 0;
  }

  memcpy(&hword, controller->ram + address, sizeof(hword));
  *data = ByteOrderSwap16(hword);

  return 0;
}

/* ============================================================================
 *  PIFRAMReadWord: Read word from PIF.
 * ========================================================================= */
int
PIFRAMReadWord(void *_controller, uint32_t address, void *_data) {
	struct PIFController *controller = (struct PIFController*) _controller;
	uint32_t *data = (uint32_t*) _data, word;

  debugarg("PIFRAMReadWord: Read from address [0x%.8X]", address);
  address = address - PIF_RAM_BASE_ADDRESS;

  if (address == 0x24)
    controller->status = 0x80;

  else if (address == 0x3C) {
    *data = controller->status;
    return 0;
  }

  memcpy(&word, controller->ram + address, sizeof(word));
  *data = ByteOrderSwap32(word);

  return 0;
}

/* ============================================================================
 *  PIFRAMWriteByte: Write byte to PIF.
 * ========================================================================= */
int
PIFRAMWriteByte(void *_controller, uint32_t address, void *_data) {
	struct PIFController *controller = (struct PIFController*) _controller;
	uint8_t *data = (uint8_t*) _data, byte;

  debugarg("PIFRAMWriteByte: Write to address [0x%.8X]", address);
  address = address - PIF_RAM_BASE_ADDRESS;

  byte = *data;
  memcpy(controller->ram + address, &byte, sizeof(byte));

  BusRaiseRCPInterrupt(controller->bus, MI_INTR_SI);
  controller->regs[SI_STATUS_REG] |= 0x1000;
  return 0;
}

/* ============================================================================
 *  PIFRAMWriteHWord: Write halfword to PIF.
 * ========================================================================= */
int
PIFRAMWriteHWord(void *_controller, uint32_t address, void *_data) {
	struct PIFController *controller = (struct PIFController*) _controller;
	uint16_t *data = (uint16_t*) _data, hword;

  debugarg("PIFRAMWriteHWord: Write to address [0x%.8X]", address);
  address = address - PIF_RAM_BASE_ADDRESS;

  hword = ByteOrderSwap16(*data);
  memcpy(controller->ram + address, &hword, sizeof(hword));

  BusRaiseRCPInterrupt(controller->bus, MI_INTR_SI);
  controller->regs[SI_STATUS_REG] |= 0x1000;
  return 0;
}

/* ============================================================================
 *  PIFRAMWriteWord: Write word to PIF.
 * ========================================================================= */
int
PIFRAMWriteWord(void *_controller, uint32_t address, void *_data) {
	struct PIFController *controller = (struct PIFController*) _controller;
	uint32_t *data = (uint32_t*) _data, word;

  debugarg("PIFRAMWriteWord: Write to address [0x%.8X]", address);
  address = address - PIF_RAM_BASE_ADDRESS;

  word = ByteOrderSwap32(*data);
  memcpy(controller->ram + address, &word, sizeof(word));

  BusRaiseRCPInterrupt(controller->bus, MI_INTR_SI);
  controller->regs[SI_STATUS_REG] |= 0x1000;
  return 0;
}

/* ============================================================================
 *  PIFROMRead: Read from PIF.
 * ========================================================================= */
int
PIFROMRead(void *_controller, uint32_t address, void *_data) {
	struct PIFController *controller = (struct PIFController*) _controller;
	uint32_t *data = (uint32_t*) _data, word;

  address = address - PIF_ROM_BASE_ADDRESS;

  memcpy(&word, controller->rom + address, sizeof(word));
  *data = ByteOrderSwap32(word);

  return 0;
}

/* ============================================================================
 *  PIFROMWrite: Write to PIF.
 * ========================================================================= */
int
PIFROMWrite(void *unused(controller), uint32_t address, void *unused(data)) {

  debugarg("PIF ROM: Detected write [0x%.8x]", address);

#ifdef NDEBUG
  address = address;
#endif
  return 0;
}

/* ============================================================================
 *  SetCICSeed: Sets the CIC seed.
 * ========================================================================= */
void
SetCICSeed(struct PIFController *pif, uint32_t seed) {
  seed = ByteOrderSwap32(seed);

  memcpy(pif->ram + 0x24, &seed, sizeof(seed));
}

/* ============================================================================
 *  SIRegRead: Read from SI registers.
 * ========================================================================= */
int
SIRegRead(void *_controller, uint32_t address, void *_data) {
	struct PIFController *controller = (struct PIFController*) _controller;
	uint32_t *data = (uint32_t*) _data;

  address -= SI_REGS_BASE_ADDRESS;
  enum SIRegister reg = (enum SIRegister) (address / 4);

  debugarg("SIRegRead: Reading from register [%s].", SIRegisterMnemonics[reg]);
  *data = controller->regs[reg];

  return 0;
}

/* ============================================================================
 *  SIRegWrite: Write to SI registers.
 * ========================================================================= */
int
SIRegWrite(void *_controller, uint32_t address, void *_data) {
	struct PIFController *controller = (struct PIFController*) _controller;
	uint32_t *data = (uint32_t*) _data;

  address -= SI_REGS_BASE_ADDRESS;
  enum SIRegister reg = (enum SIRegister) (address / 4);

  debugarg("SIRegWrite: Writing to register [%s].", SIRegisterMnemonics[reg]);

  if (reg == SI_STATUS_REG) {
    BusClearRCPInterrupt(controller->bus, MI_INTR_SI);
    controller->regs[SI_STATUS_REG] &= ~0x1000;
  }

  else if (reg == SI_PIF_ADDR_RD64B_REG)
    SIHandleDMARead(controller);
  else if (reg == SI_PIF_ADDR_WR64B_REG)
    SIHandleDMAWrite(controller);

  else
    controller->regs[reg] = *data;

  return 0;
}

