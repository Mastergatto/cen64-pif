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

#include <GL/glfw.h>

/* ============================================================================
 *  CalculateMemPakCRC: Calculates the CRC of MemPak data.
 *  TODO: Ripped straight from MAME/MESS; look into it.
 * ========================================================================= */
static uint8_t
CalculateMemPakCRC(uint8_t *buffer, int size) {
  uint32_t crc = 0;
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

#ifdef RETROLINK_JOYSTICK
  unsigned char buttons[12];
  float joystick[2];
  int8_t axes[2];
#endif

  switch(command) {
  case 0x00:
  case 0xFF:
    debug("Command: Read PIF status/reset?");

    switch(channel) {
    case 0:
      recvBuffer[0] = 0x05;
      recvBuffer[1] = 0x00;
      recvBuffer[2] = 0x01;
      break;

    case 1:
    case 2:
    case 3:
      return 1;

    case 4:
      recvBuffer[0] = 0x00;
      recvBuffer[1] = 0x80;
      recvBuffer[2] = 0x00;
      break;

    default:
      debug("PIF: Status/reset on unknown channel?");
      return 1;
    }

    break;

  case 0x01:
    switch(channel) {
    case 0:
      debug("Read from P1 controller.");
      recvBuffer[0] = 0x00;
      recvBuffer[1] = 0x00;
      recvBuffer[2] = 0x00;
      recvBuffer[3] = 0x00;

#ifdef RETROLINK_JOYSTICK
      /* Read the x and y axes of the controller. */
      glfwGetJoystickPos(GLFW_JOYSTICK_1, joystick, 2);
      glfwGetJoystickButtons(GLFW_JOYSTICK_1, buttons, 12);

      axes[0] = joystick[0] * 127;
      axes[1] = joystick[1] * 127;
      recvBuffer[2] = axes[0];
      recvBuffer[3] = axes[1];

      /* Check for joystick input. */
      recvBuffer[0] |= (axes[0] < 0) ? (unsigned) BUTTON_JOY_LEFT >> 8 : 0;
      recvBuffer[0] |= (axes[0] > 0) ? (unsigned) BUTTON_JOY_RIGHT >> 8 : 0;
      recvBuffer[0] |= (axes[1] < 0) ? (unsigned) BUTTON_JOY_DOWN >> 8 : 0;
      recvBuffer[0] |= (axes[1] > 0) ? (unsigned) BUTTON_JOY_UP >> 8 : 0;

      /* Check for C buttons. */
      recvBuffer[1] |= buttons[0] << 3;
      recvBuffer[1] |= buttons[1] << 0;
      recvBuffer[1] |= buttons[2] << 2;
      recvBuffer[1] |= buttons[3] << 1;

      /* Check for L/R flippers. */
      recvBuffer[1] |= buttons[4] << 5;
      recvBuffer[1] |= buttons[5] << 4;

      /* Check for A, Z, and B buttons. */
      recvBuffer[0] |= buttons[6] << 7;
      recvBuffer[0] |= buttons[7] << 5;
      recvBuffer[0] |= buttons[8] << 6;

      /* Check for the start button. */
      recvBuffer[0] |= buttons[9] << 4;
#endif

      break;

    case 1:
      debug("Read from P2 controller.");
      return 1;

    case 2:
      debug("Read from P3 controller.");
      return 1;

    case 3:
      debug("Read from P4 controller.");
      return 1;

    default:
      debug("Read from invalid controller?");
      return 1;
    }

    break;

  case 0x02:
    debug("MemPak | Command: Read from MemPak.");
    memcpy(&address, sendBuffer + 1, sizeof(address));
    address = ByteOrderSwap16(address) & ~0x1F;

    if (address == 0x8000) {
      memset(recvBuffer, 0, recvBytes - 1);
      recvBuffer[recvBytes - 1] = CalculateMemPakCRC(
        recvBuffer, recvBytes - 1);

      break;
    }

    else if (address < 0x7FE0) {
      memset(recvBuffer, 0, recvBytes - 1);
      recvBuffer[recvBytes - 1] = CalculateMemPakCRC(
        recvBuffer, recvBytes - 1);

      break;
    }

    return 1;

  case 0x03:
    debug("MemPak | Command: Write to MemPak.");
    memcpy(&address, sendBuffer + 1, sizeof(address));
    address = ByteOrderSwap16(address) & ~0x1F;

#if 0
    if (address < 0x8000) {
      /* ... */
    }
#endif

    debugarg("MemPak | Destination: [0x%.4X].", address);
    recvBuffer[0] = CalculateMemPakCRC(sendBuffer + 3, sendBytes - 3);
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
  int ptr = 0;

  if (controller->command[0x3F] != 0x1)
    return;

  /* Logic ripped from MAME. */
  while (ptr < 0x3F) {
    int8_t sendBytes = controller->command[ptr++];

    if (sendBytes == -2)
      break;

    if (sendBytes < 0)
      continue;

    if (sendBytes > 0 && (sendBytes & 0xC0) == 0) {
      int8_t recvBytes = controller->command[ptr++];
      uint8_t recvBuffer[0x40];
      uint8_t sendBuffer[0x40];
      int result;

      if (recvBytes == -2)
        break;

      memcpy(sendBuffer, controller->command + ptr, sendBytes);
      ptr += sendBytes;

      result = PIFHandleCommand(channel, sendBuffer,
        sendBytes, recvBuffer, recvBytes);

      if (result == 0) {
        if (recvBytes >= 0 && recvBytes < 64 - ptr) {
          memcpy(controller->ram + ptr, recvBuffer, recvBytes);
          ptr += recvBytes;
        }
      }

      else if (result == 1)
        controller->ram[ptr - 2] |= 0x80;
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
  memcpy(controller->command, controller->ram, 64);

  controller->regs[SI_STATUS_REG] |= 0x1000;
  BusRaiseRCPInterrupt(controller->bus, MI_INTR_SI);
}

