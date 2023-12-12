#ifndef _LIBNEWTON_CORE_VARIABLES_H__
#define _LIBNEWTON_CORE_VARIABLES_H__

#ifdef __cplusplus
#warning "This is a C library, use cpplibnewton instead"
#endif

#include "newton/defs.h"

#include <stdint.h>

typedef struct {
  const uint8_t NewtonVersion;
  uint8_t BufferSize;
} NewtonVariables;

static inline NewtonVariables Newton_CreateVariables(uint8_t BufferSize) {
  return (NewtonVariables){.NewtonVersion = LIBNEWTON_VERSION,
                           .BufferSize = BufferSize};
}

typedef enum {
  NEWTON_VARIABLES_V = 0x00,
  NEWTON_VARIABLES_S = 0x01,
  INVALID_VARIABLE = 0xFF,
} NewtonVariablesCodes;

#endif
