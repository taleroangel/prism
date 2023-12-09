#ifndef __LIBNEWTON_REGISTERS_H__
#define __LIBNEWTON_REGISTERS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Newton registers structures X and Y
 *
 */
typedef struct {
  uint8_t X;
  uint8_t Y;
} NewtonRegisters;

#ifdef __cplusplus
}
#endif

#endif
