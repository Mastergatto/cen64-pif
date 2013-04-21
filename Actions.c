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
 *  CalculateMemPakCRC: Calculates the CRC of MemPak data.
 *  TODO: Ripped straight from MAME/MESS; look into it.
 * ========================================================================= */
static uint8_t
CalculateMemPakCRC(uint8_t *buffer, int size) {
  uint32_t crc;
  int i, j;

  for (i = 0; i <= size; i++) {
    for (j = 7; j >= 0; j--) {
      uint32_t temp = ((crc & 0x80) != 0) ? 0x85 : 0x00;

      crc <<= 1;

      if (i == size)
        crc &= 0xFF;

      else {
        if ((buffer[i] & (1 << j)) != 0)
          crc |= 0x1;
      }

      crc ^= temp;
    }
  }

  return crc;
}

/* ============================================================================
 *  PIFHandleCommand: Perform action specified by the PIF RAM.
 *  TODO: Ripped straight from MAME/MESS; look into it.
 * ========================================================================= */
static int
PIFHandleCommand(unsigned channel, uint8_t *sendBuffer,
  uint8_t sendBytes, uint8_t *recvBuffer, uint8_t recvBytes) {
  uint8_t command = sendBuffer[0];
  uint16_t address;

  switch(command) {
  case 0x00:
    debug("Command: Read PIF status.");
    break;

  case 0x01:
    switch(channel) {
    case 0:
      debug("Read from P1 controller.");
      break;

    case 1:
      debug("Read from P2 controller.");
      break;

    case 2:
      debug("Read from P3 controller.");
      break;

    case 3:
      debug("Read from P4 controller.");
      break;

    default:
      debug("Read from invalid controller?");
    }

    break;

  case 0x03:
    debug("MemPak | Command: Write to MemPak.");
    memcpy(&address, sendBuffer + 1, sizeof(address));
    address = ByteOrderSwap16(address) & ~0x1F;

    if (address > 0x8000) {
      debug("MemPak: Write to invalid address.");
      break;
    }

    debugarg("MemPak | Destination: [0x%.4X].", address);
    recvBuffer[0] = CalculateMemPakCRC(sendBuffer + 3, sendBytes - 3);
    break;

  case 0xFF:
    debug("Command: Reset.");
    break;

  default:
    debugarg("Unimplemented command: [0x%.2X].", command);
  }

  return 0;
}

static void PIFProcess(struct PIFController *);

/* ============================================================================
 *  PIFProcess: Perform action specified by the PIF RAM.
 *  TODO: Ripped straight from MAME/MESS; look into it.
 * ========================================================================= */
static void
PIFProcess(struct PIFController *controller) {
  unsigned channel = 0;
  unsigned ptr = 0;

  if (controller->ram[0x3F] != 0x1)
    return;

  /* Logic ripped from MAME. */
  while (ptr < 0x3F) {
    int8_t sendBytes = controller->ram[ptr++];

    if (sendBytes == -2)
      break;

    if (sendBytes < 0)
      continue;

    if (sendBytes > 0 && (sendBytes & 0xC0) == 0) {
      int8_t recvBytes = controller->ram[ptr++];
      uint8_t recvBuffer[0x40];
      uint8_t sendBuffer[0x40];
      int result;

      if (recvBytes == -2)
        break;

      memcpy(sendBuffer, controller->ram + ptr, sendBytes);
      ptr += sendBytes;

      result = PIFHandleCommand(channel, sendBuffer,
        sendBytes, recvBuffer, recvBytes);

      if (result == 0) {
        memcpy(controller->ram + ptr, recvBuffer, recvBytes);
        ptr += recvBytes;
      }

      else if (result == 1) {
        controller->ram[ptr] |= 0x80;
      }
    }

    channel++;
  }

  controller->ram[0x3F] = 0;
}

/* ============================================================================
 *  SIHandleDMARead: Invoked when SI_PIF_ADDR_RD64B_REG is written.
 *
 *  PIF RAM = Source.
 *  SI_DRAM_ADDR_REG = DRAM (target) address.
 *  64 bytes = Transfer size.
 * ========================================================================= */
void
SIHandleDMARead(struct PIFController *controller) {
  uint32_t target = controller->regs[SI_DRAM_ADDR_REG] & 0x1FFFFFFF;
  assert(((target & 0x3) == 0) && "Unaligned access.");

  PIFProcess(controller);

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
void
SIHandleDMAWrite(struct PIFController *controller) {
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

