#ifndef __LIBNEWTON_CORE_INSTRUCTION_HXX__
#define __LIBNEWTON_CORE_INSTRUCTION_HXX__

#ifndef __cplusplus
#error "This library is not C compatible, use clibnewton instead"
#endif

#include <cstdint>

namespace newton::core {
/**
 * @brief Prism instruction set
 *
 */
enum [[gnu::packed]] InstructionSet {
  /* Control instructions */
  UPDATE = 0x00,
  CLEAR = 0x01,
  SLEEP = 0x07,

  /* Selection instructions */
  SELECT = 0x02,
  RANGE = 0x03,

  /* Buffer alteration instructions */
  SET = 0x04,
  FILL = 0x05,

  /* Effects */
  BLUR = 0x06,

  /* Register instructions */
  LOADX = 0x08,
  LOADY = 0x09,

  /* Misc */
  IGNORE_INSTRUCTION = 0xFE,
  EXCEPTION = 0xFF,
};

union [[gnu::packed]] InstructionOptions {

  struct [[gnu::packed, gnu::aligned(sizeof(uint8_t))]] {
    volatile uint8_t _ = 0x00;
  } NoOptions;

  enum [[gnu::packed]] {
    OPTION_SELECT_ABSOLUTE = 0,
    OPTION_SELECT_RELATIVE = 1
  } SelectOptions;

  enum [[gnu::packed]] {
    OPTION_RANGE_ABSOLUTE_START = 0,
    OPTION_RANGE_RELATIVE_START = 1,
    OPTION_RANGE_ABSOLUTE_END = 2,
    OPTION_RANGE_RELATIVE_END = 3,
  } RangeOptions;

  enum [[gnu::packed]] {
    OPTION_COLOR_RED = 0,
    OPTION_COLOR_GREEN = 1,
    OPTION_COLOR_BLUE = 2,
  } ColorOptions;

  enum [[gnu::packed]] {
    OPTION_APPLY_ALL = 0,
    OPTION_APPLY_RANGE = 1,
  } EffectOptions;

  enum [[gnu::packed]] {
    OPTION_TIME_MS = 1,
    OPTION_TIME_SEC = 2,
    OPTION_TIME_MIN = 3,
  } TimeOptions;

  enum [[gnu::packed]] {
    OPTION_LOAD_VARIABLE = 0,
    OPTION_LOAD_R = 1,
    OPTION_LOAD_G = 2,
    OPTION_LOAD_B = 3,
  } LoadOptions;
};

/**
 * @brief Prism parsed instruction
 *
 */
struct Instruction {
  InstructionSet instruction;
  InstructionOptions options = {};
  uint8_t value = 0x00;
};

/**
 * @brief Instruction byte-fields definition
 * @note Endianess dependent
 */
struct [[gnu::packed, gnu::aligned(sizeof(uint8_t))]] _RawInstruction {
  uint8_t instruction : 6;
  uint8_t options : 2;
  uint8_t value;
};

} // namespace newton::core

#endif
