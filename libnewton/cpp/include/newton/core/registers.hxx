#ifndef __LIBNEWTON_CORE_REGISTERS_HXX__
#define __LIBNEWTON_CORE_REGISTERS_HXX__

#include "newton/core/variables.hxx"
#ifndef __cplusplus
#error "This library is not C compatible, use clibnewton instead"
#endif

#include <cstdint>

namespace newton::core {
/**
 * @brief Newton registers structures X and Y
 *
 */
struct Registers {
  uint8_t X;
  uint8_t Y;

  Registers() = delete;
  inline Registers(const Variables variables)
      : X(variables.NewtonVersion), Y(variables.BufferSize) {}
};

} // namespace newton::core

#endif
