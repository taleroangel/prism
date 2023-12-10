#ifndef __LIBNEWTON_CORE_VARIABLES_H__
#define __LIBNEWTON_CORE_VARIABLES_H__

#ifndef __cplusplus
#error "This library is not C compatible, use clibnewton instead"
#endif

#include "../defs.h"

#include <cstdint>

namespace newton::core {

struct Variables {
  const uint8_t NewtonVersion = LIBNEWTON_VERSION;
  uint8_t BufferSize;

  Variables() = delete;
  inline Variables(uint8_t BufferSize):
  	BufferSize(BufferSize), NewtonVersion(LIBNEWTON_VERSION) {}
};

enum VariablesCodes {
  NEWTON_VARIABLES_V = 0x00,
  NEWTON_VARIABLES_S = 0x01,
  INVALID_VARIABLE = 0xFF,
};

}

#endif
