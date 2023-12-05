#include "newton/instruction.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <newton.h>

#if !defined(__GNUC__) || defined(NDEBUG)
#error "GNU extensions are required and NDEBUG must not be defined"
#endif

void assert_instruction(uint16_t raw, PrismInstructionSet instruction,
                        PrismInstructionOptions options, uint8_t value);

int main() {
  // Update
  assert_instruction(0b0000000011111111, PRISM_INSTRUCTION_UPDATE,
                     (PrismInstructionOptions){.NoOptions = {}}, 0xFF);

  // Clear
  assert_instruction(0b0000010011111111, PRISM_INSTRUCTION_CLEAR,
                     (PrismInstructionOptions){.NoOptions = {}}, 0xFF);

  // Select absolute
  assert_instruction(
      0b0000100010101010, PRISM_INSTRUCTION_SELECT,
      (PrismInstructionOptions){.SelectOptions = PRISM_OPTION_SELECT_ABSOLUTE},
      0xAA);

  // Select relative
  assert_instruction(
      0b0000100110101010, PRISM_INSTRUCTION_SELECT,
      (PrismInstructionOptions){.SelectOptions = PRISM_OPTION_SELECT_RELATIVE},
      0xAA);

  // Range absolute start
  assert_instruction(0b0000110010101010, PRISM_INSTRUCTION_RANGE,
                     (PrismInstructionOptions){
                         .RangeOptions = PRISM_OPTION_RANGE_ABSOLUTE_START},
                     0xAA);

  // Range absolute end
  assert_instruction(0b0000111011111111, PRISM_INSTRUCTION_RANGE,
                     (PrismInstructionOptions){
                         .RangeOptions = PRISM_OPTION_RANGE_ABSOLUTE_END},
                     0xFF);

  // Range relative start
  assert_instruction(0b0000110110101010, PRISM_INSTRUCTION_RANGE,
                     (PrismInstructionOptions){
                         .RangeOptions = PRISM_OPTION_RANGE_RELATIVE_START},
                     0xAA);

  // Range relative end
  assert_instruction(0b0000111111111111, PRISM_INSTRUCTION_RANGE,
                     (PrismInstructionOptions){
                         .RangeOptions = PRISM_OPTION_RANGE_RELATIVE_END},
                     0xFF);

  // Fill Color R/H
  assert_instruction(
      0b0001010011110000, PRISM_INSTRUCTION_FILL,
      (PrismInstructionOptions){.ColorOptions = PRISM_OPTION_COLOR_RED}, 0xF0);

  // Fill Color G/S
  assert_instruction(
      0b0001010111110000, PRISM_INSTRUCTION_FILL,
      (PrismInstructionOptions){.ColorOptions = PRISM_OPTION_COLOR_GREEN},
      0xF0);

  // Fill Color B/V
  assert_instruction(
      0b0001011011110000, PRISM_INSTRUCTION_FILL,
      (PrismInstructionOptions){.ColorOptions = PRISM_OPTION_COLOR_BLUE}, 0xF0);

  // Set Color R/H
  assert_instruction(
      0b0001000011110000, PRISM_INSTRUCTION_SET,
      (PrismInstructionOptions){.ColorOptions = PRISM_OPTION_COLOR_RED}, 0xF0);

  // Set Color G/S
  assert_instruction(
      0b0001000111110000, PRISM_INSTRUCTION_SET,
      (PrismInstructionOptions){.ColorOptions = PRISM_OPTION_COLOR_GREEN},
      0xF0);

  // Set Color B/V
  assert_instruction(
      0b0001001011110000, PRISM_INSTRUCTION_SET,
      (PrismInstructionOptions){.ColorOptions = PRISM_OPTION_COLOR_BLUE}, 0xF0);

  // Blur All
  assert_instruction(
      0b0001100011110000, PRISM_INSTRUCTION_BLUR,
      (PrismInstructionOptions){.EffectOptions = PRISM_OPTION_APPLY_ALL}, 0xF0);

  // Blur Range
  assert_instruction(
      0b0001100111110000, PRISM_INSTRUCTION_BLUR,
      (PrismInstructionOptions){.EffectOptions = PRISM_OPTION_APPLY_RANGE},
      0xF0);

  // Sleep (us)
  assert_instruction(
      0b0001110011110000, PRISM_INSTRUCTION_SLEEP,
      (PrismInstructionOptions){.TimeOptions = PRISM_OPTION_TIME_US}, 0xF0);

  // Sleep (ms)
  assert_instruction(
      0b0001110111110000, PRISM_INSTRUCTION_SLEEP,
      (PrismInstructionOptions){.TimeOptions = PRISM_OPTION_TIME_MS}, 0xF0);

  // Sleep (sec)
  assert_instruction(
      0b0001111011110000, PRISM_INSTRUCTION_SLEEP,
      (PrismInstructionOptions){.TimeOptions = PRISM_OPTION_TIME_SEC}, 0xF0);

  // Sleep (min)
  assert_instruction(
      0b0001111111110000, PRISM_INSTRUCTION_SLEEP,
      (PrismInstructionOptions){.TimeOptions = PRISM_OPTION_TIME_MIN}, 0xF0);
}

void assert_instruction(uint16_t raw, PrismInstructionSet instruction,
                        PrismInstructionOptions options, uint8_t value) {
  // Parse the instruction from U16
  PrismInstruction parsed = Newton_ParseInstructionU16(raw);

  // Assert instruction type
  assert(parsed.instruction == instruction);
  // Assert options (bit by bit)
  assert(*((uint8_t *)&parsed.options) == *((uint8_t *)&options));
  // Assert value (bit by bit)
  assert(parsed.value == value);
}
