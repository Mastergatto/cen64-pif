/* ============================================================================
 *  Registers.md: SI registers.
 *
 *  PIFSIM: Peripheral InterFace SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef SI_REGISTER_LIST
#define SI_REGISTER_LIST \
  Y(SI_DRAM_ADDR_REG) \
  Y(SI_PIF_ADDR_RD64B_REG) \
  Y(SI_RESERVED_1_REG) \
  Y(SI_RESERVED_2_REG) \
  Y(SI_PIF_ADDR_WR64B_REG) \
  Y(SI_RESERVED_3_REG) \
  Y(SI_STATUS_REG)
#endif

SI_REGISTER_LIST

