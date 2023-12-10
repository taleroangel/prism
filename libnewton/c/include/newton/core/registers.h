#ifndef _LIBNEWTON_CORE_REGISTERS_H__
#define _LIBNEWTON_CORE_REGISTERS_H__

#ifdef __cplusplus
#warning "This is a C library, use cpplibnewton instead"
#endif

#include "newton/core/variables.h"
#include <stdint.h>

/**
 * @brief Newton registers structures X and Y
 *
 */
typedef struct {
  uint8_t X;
  uint8_t Y;
} NewtonRegisters;

/**
 * @brief Create registers with default values
 * @note \struct{NewtonVariables} must be already set
 * @return NewtonRegisters
 */
static inline NewtonRegisters
Newton_InitRegisters(const NewtonVariables variables) {
  return (NewtonRegisters){.X = variables.NewtonVersion,
                           .Y = variables.BufferSize};
}

#endif
