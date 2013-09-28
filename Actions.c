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
#include <cstddef>
#include <cstring>
#else
#include <assert.h>
#include <stddef.h>
#include <string.h>
#endif

#ifdef GLFW3
#include <GLFW/glfw3.h>
#else
#include <GL/glfw.h>
#endif

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
PIFHandleCommand(struct PIFController *controller, unsigned channel,
  uint8_t *sendBuffer, uint8_t sendBytes, uint8_t *recvBuffer,
  uint8_t recvBytes) {
  uint8_t command = sendBuffer[0];
  uint16_t address, offset;

#ifdef GLFW3
  int count;
  const unsigned char *buttons;
  const float *joystick;
#else
  unsigned char buttons[16];
  float joystick[5];
  uint32_t joystickint[5];
#endif /*GLFW3*/
  int8_t axes[2];
  uint8_t shift;
  
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
    case 1:
    case 2:
    case 3:
      debug("Read from controller.");
      recvBuffer[0] = 0x00;
      recvBuffer[1] = 0x00;
      recvBuffer[2] = 0x00;
      recvBuffer[3] = 0x00;
      /* do shit for controller*/
      switch(controller->input) {
      case KEYBOARD:
          /* Joystick, shift+direction keys*/
          shift = (glfwGetKey(GLFW_KEY_LSHIFT) | glfwGetKey(GLFW_KEY_RSHIFT) ? 114 : 38);
          if(glfwGetKey(GLFW_KEY_UP))           recvBuffer[3] = glfwGetKey(GLFW_KEY_UP) * shift;
          else if(glfwGetKey(GLFW_KEY_DOWN))    recvBuffer[3] = glfwGetKey(GLFW_KEY_DOWN) * -shift;
          if(glfwGetKey(GLFW_KEY_RIGHT))        recvBuffer[2] = glfwGetKey(GLFW_KEY_RIGHT) * shift;
          else if(glfwGetKey(GLFW_KEY_LEFT))    recvBuffer[2] = glfwGetKey(GLFW_KEY_LEFT) * -shift;
          /* Check for C buttons */
          recvBuffer[1] |= glfwGetKey(GLFW_KEY_HOME) << 3; // C Up
          recvBuffer[1] |= glfwGetKey(GLFW_KEY_END) << 2; // C Down
          recvBuffer[1] |= glfwGetKey(GLFW_KEY_DEL) << 1; // C Left
          recvBuffer[1] |= glfwGetKey(GLFW_KEY_PAGEDOWN) << 0; // C Right
          /* Check for L/R flippers. */
          recvBuffer[1] |= glfwGetKey('A') << 5;
          recvBuffer[1] |= glfwGetKey('S')  << 4;
          /* Check for A, Z, and B buttons. */
          recvBuffer[0] |= glfwGetKey('X') << 7; // A
          recvBuffer[0] |= glfwGetKey('C') << 6; // B
          recvBuffer[0] |= glfwGetKey('Z') << 5; // Z
          recvBuffer[0] |= glfwGetKey(GLFW_KEY_ENTER) << 4; // Start
          /* Check for the D-Pad buttons. */
          recvBuffer[0] |= glfwGetKey(GLFW_KEY_KP_8) << 3; // D Up
          recvBuffer[0] |= glfwGetKey(GLFW_KEY_KP_2) << 2; // D Down
          recvBuffer[0] |= glfwGetKey(GLFW_KEY_KP_4) << 1; // D Left
          recvBuffer[0] |= glfwGetKey(GLFW_KEY_KP_6) << 0; // D Right
          break;


      case RETROLINK:
          /* Read the x and y axes of the controller. */
          glfwGetJoystickPos(channel, joystick, 2);
          glfwGetJoystickButtons(channel, buttons, 12);

          axes[0] = joystick[0] * 127;
          axes[1] = joystick[1] * 127;
          recvBuffer[2] = axes[0];
          recvBuffer[3] = axes[1];

          /* Check for joystick input. */
          recvBuffer[0] = 0;

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
          break;

      case MAYFLASH_N64:
          /* Read the x and y axes of the controller. */
          glfwGetJoystickPos(channel, joystick, 4);
          glfwGetJoystickButtons(channel, buttons, 16);

          axes[0] = joystick[0] * 127;
          axes[1] = joystick[1] * 127;
          recvBuffer[2] = axes[0];
          recvBuffer[3] = axes[1];

          /* Check for joystick input. */
          recvBuffer[0] = 0;

          /* Check for C buttons. */
          // if you *still* don't like this code, then eat a dick
          memcpy(joystickint, joystick, 16);
          if(0x3F4103C2 == joystickint[2]) recvBuffer[1] |= BUTTON_C_DOWN;
          else if(0xBF3FFFC0 == joystickint[2]) recvBuffer[1] |= BUTTON_C_UP;
          if(0x3F4103C2 == joystickint[3]) recvBuffer[1] |= BUTTON_C_LEFT;
          else if(0xBF3FFFC0 == joystickint[3]) recvBuffer[1] |= BUTTON_C_RIGHT;

          /* Check for L/R flippers. */
          recvBuffer[1] |= buttons[6] << 5;
          recvBuffer[1] |= buttons[7] << 4;

          /* Check for A, B, Z, and Start buttons. */
          recvBuffer[0] |= buttons[1] << 7; // A
          recvBuffer[0] |= buttons[2] << 6; // B
          recvBuffer[0] |= buttons[8] << 5; // Z
          recvBuffer[0] |= buttons[9] << 4; // S

          /* Check for the D-Pad buttons. */
          recvBuffer[0] |= buttons[12] << 3; // D Up
          recvBuffer[0] |= buttons[14] << 2; // D Down
          recvBuffer[0] |= buttons[15] << 1; // D Left
          recvBuffer[0] |= buttons[13] << 0; // D Right
          break;

      case XBOX360:
          // joystick[0] = left joystick X, left=-1.000 - right=+1.000
          // joystick[1] = left joystick Y, down=-1.000 - up   =+1.000 *(unsigned int*)&
          // joystick[3] = right joystick Y, up =-1.000 - down =+1.000
          // joystick[4] = right joystick X,left=-1.000 - right=+1.000 *(unsigned int*)&
          // button 0 = A, 1 = B, 2 = X, 3 = Y, 4 = L, 5 = R, 6 = BACK, 7 = START, 8 = LEFT ANALOG CLICK, 9 = RIGHT ANALOG CLICK
          glfwGetJoystickPos(channel, joystick, 5);
          glfwGetJoystickButtons(channel, buttons, 10);

          axes[0] = joystick[0] * 127;
          axes[1] = joystick[1] * 127;
          recvBuffer[2] = axes[0];
          recvBuffer[3] = axes[1];

          /* Check for joystick input. */
          recvBuffer[0] = 0;
          recvBuffer[1] = 0;
          /* Check for C buttons. */
          if(joystick[3] > .75F) recvBuffer[1] |= BUTTON_C_DOWN;
          else if(joystick[3] < -.75F) recvBuffer[1] |= BUTTON_C_UP;
          if(joystick[4] < -.75F) recvBuffer[1] |= BUTTON_C_LEFT;
          else if(joystick[4] > .75F) recvBuffer[1] |= BUTTON_C_RIGHT;

          /* Check for L/R flippers. */
          recvBuffer[1] |= buttons[4] << 5; // L
          recvBuffer[1] |= (joystick[2] < -.75F) ? (1 << 4) : 0; // R

          /* Check for A, B, Z, and Start buttons. */
          recvBuffer[0] |= buttons[0] << 7; // A
          recvBuffer[0] |= buttons[1] << 6; // B
          recvBuffer[0] |= (joystick[2] > .75F) ? (1 << 5) : 0; // Z
          recvBuffer[0] |= buttons[7] << 4; // S

          /* Check for the D-Pad buttons. */
          /*recvBuffer[0] |= buttons[12] << 3; // D Up
          recvBuffer[0] |= buttons[14] << 2; // D Down
          recvBuffer[0] |= buttons[15] << 1; // D Left
          recvBuffer[0] |= buttons[13] << 0; // D Right */

      default:
        break;
      }
      break;

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

  case 0x04:
    debug("EEPROM | Command: Read from EEPROM.");

    if (channel != 4)
      return 1;

    if (sendBytes != 2 || recvBytes != 8) {
      debug("EEPROM | Unusual send/recv sizes?");
    }

    offset = sendBuffer[1] * 8;
    memcpy(recvBuffer, controller->eeprom + offset, 8);
    break;

  case 0x05:
    debug("EEPROM | Command: Write to EEPROM.");

    if (channel != 4)
      return 1;

    if (sendBytes != 10 || recvBytes != 1) {
      debug("EEPROM | Unusual send/recv sizes?");
    }

    offset = sendBuffer[1] * 8;
    memcpy(controller->eeprom + offset, sendBuffer + 2, 8);
    break;

  default:
    debugarg("Unimplemented command: [0x%.2X].", command);

    return 1;
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

      result = PIFHandleCommand(controller, channel,
        sendBuffer, sendBytes, recvBuffer, recvBytes);

      if (result == 0) {
        memcpy(controller->ram + ptr, recvBuffer, recvBytes);
        ptr += recvBytes;
      }

      else if (result == 1)
        controller->ram[ptr - 2] |= 0x80;
    }

    channel++;
  }

  controller->ram[0x3F] = 0;
}

/* ============================================================================
 *  ReadEEPROMFile: Reads the contents EEPROM file into the controller.
 * ========================================================================= */
int
ReadEEPROMFile(struct PIFController *controller) {
  size_t cur = 0;

  if (!controller->eepromFile)
    return -1;

  rewind(controller->eepromFile);

  while (cur < sizeof(controller->eeprom)) {
    size_t remaining = sizeof(controller->eeprom) - cur;
    size_t ret;

    if ((ret = fread(controller->eeprom + cur, 1,
      remaining, controller->eepromFile)) == 0 &&
      ferror(controller->eepromFile))
      return -1;

    /* Ignore invalid sized files. */
    if (feof(controller->eepromFile)) {
      memset(controller->eeprom, 0, sizeof(controller->eeprom));
      printf("EEPROM: Ignoring short EEPROM file.\n");
      return 0;
    }

    cur += ret;
  }

  return 0;
}

/* ============================================================================
 *  SetControlType: Sets the Control Type for the emulator.
 * ========================================================================= */
void
SetControlType(struct PIFController *controller, const char *controltype) {
  controller->input = INVALID;

  if(!strncmp("keyboard", controltype, 8))
    controller->input = KEYBOARD;
  else if(!strncmp("mayflash64", controltype, 10))
    controller->input = MAYFLASH_N64;
  else if(!strncmp("retrolink", controltype, 9))
    controller->input = RETROLINK;
  else if(!strncmp("x360", controltype, 4))
    controller->input = XBOX360;

  /* Default to keyboard. */
  if (controller->input == INVALID)
    controller->input = KEYBOARD;
}

/* ============================================================================
 *  SetEEPROMFile: Sets the backing file for EEPROM saves.
 * ========================================================================= */
void
SetEEPROMFile(struct PIFController *controller, const char *filename) {
  if (controller->eepromFile != NULL)
    fclose(controller->eepromFile);

  /* Try opening with rb+ first, then wb+ iff we fail. */
  if ((controller->eepromFile = fopen(filename, "rb+")) == NULL) {
    controller->eepromFile = fopen(filename, "wb+");
    return;
  }

  ReadEEPROMFile(controller);
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

/* ============================================================================
 *  WriteEEPROMFile: Dumps the contents of the EEPROM to the backing file.
 * ========================================================================= */
int
WriteEEPROMFile(struct PIFController *controller) {
  size_t cur = 0;

  if (!controller->eepromFile)
    return -1;

  rewind(controller->eepromFile);

  while (cur < sizeof(controller->eeprom)) {
    size_t remaining = sizeof(controller->eeprom) - cur;
    size_t ret;

    if ((ret = fwrite(controller->eeprom + cur, 1,
      remaining, controller->eepromFile)) == 0 &&
      ferror(controller->eepromFile))
      return -1;

    cur += ret;
  }

  return 0;
}

