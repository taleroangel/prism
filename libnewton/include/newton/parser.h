#ifndef __LIBNEWTON_PARSER_H__
#define __LIBNEWTON_PARSER_H__

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "instruction.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Parse a 16 byte integer (Little Endian) into a
 * \struct{PrismInstruction}
 *
 * @param byte Raw 16 bytes in LittleEndian format
 * @return PrismInstruction Parsed instruction
 */
static inline PrismInstruction Newton_ParseInstructionU16(uint16_t byte) {
  _NewtonRawInstruction *rawInstruction = (_NewtonRawInstruction *)(&byte);

  PrismInstruction parsedInstruction = {
      .instruction = (PrismInstructionSet)rawInstruction->instruction,
      .value = rawInstruction->value};

  uint8_t optionsValue = rawInstruction->options;
  memcpy(&parsedInstruction.options, &optionsValue, sizeof(uint8_t));

  return parsedInstruction;
}

/**
 * @brief Parse an instruction literal into a \struct{PrismInstruction}
 *
 * @param instruction Literal with the instruction without \\n character
 * @return PrismInstruction Built Parsed instruction
 */
static inline PrismInstruction
Newton_ParseInstructionLiteral(const char *instruction) {

  // If instruction pointer is null return exception
  if (instruction == NULL) {
    return (PrismInstruction){.instruction = PRISM_EXCEPTION,
                              .options = {.NoOptions = {}}};
  }

  // Contains comment or empty line (Ignore instruction)
  if ((strncmp(instruction, "--", 2) == 0) || (strlen(instruction) <= 2)) {
    return (PrismInstruction){.instruction = PRISM_IGNORE_INSTRUCTION,
                              .options = {.NoOptions = {}}};
  }

  // Generate the instruction
  PrismInstruction parsedInstruction = (PrismInstruction){
      .instruction = PRISM_IGNORE_INSTRUCTION, .options = {.NoOptions = {}}};

  // Tokenize instructions
  char *token = strtok((char *)instruction, " ");

  /* Ignore instruction */
  if (strcmp(token, "NOP") == 0) {
    ; /* Do nothing */
  }

  /* Update the buffer */
  else if (strcmp(token, "UPDT") == 0) {
    parsedInstruction.instruction = PRISM_INSTRUCTION_UPDATE;
  }

  /* Clear the buffer */
  else if (strcmp(token, "CLR") == 0) {
    parsedInstruction.instruction = PRISM_INSTRUCTION_CLEAR;
  }

  /* Select a value from buffer */
  else if (strcmp(token, "SEL") == 0) {
    parsedInstruction.instruction = PRISM_INSTRUCTION_SELECT;

    // Next token is mode
    token = strtok(NULL, " ");

    switch (token[0]) {
      /* For relative selection */
    case '%':
      parsedInstruction.options =
          (PrismInstructionOptions){.SelectOptions =
#ifdef __cplusplus
                                        PrismInstructionOptions::
#endif
                                            PRISM_OPTION_SELECT_RELATIVE};
      break;

      /* For absolute selection */
    case '#':
      parsedInstruction.options =
          (PrismInstructionOptions){.SelectOptions =
#ifdef __cplusplus
                                        PrismInstructionOptions::
#endif
                                            PRISM_OPTION_SELECT_ABSOLUTE};
      break;
    default:
      return (PrismInstruction){.instruction = PRISM_EXCEPTION,
                                .options = {.NoOptions = {}}};
    }

    // Next token is value
    token = strtok(NULL, " ");
    parsedInstruction.value = (uint8_t)strtol(token, NULL, 16);
  }

  /* Select a range from buffer */
  else if (strcmp(token, "RAN") == 0) {
    parsedInstruction.instruction = PRISM_INSTRUCTION_RANGE;
    token = strtok(NULL, " ");

    if (strcmp(token, "%") == 0) {
      parsedInstruction.options =
          (PrismInstructionOptions){.RangeOptions =
#ifdef __cplusplus
                                        PrismInstructionOptions::
#endif
                                            PRISM_OPTION_RANGE_RELATIVE_START};
    } else if (strcmp(token, "%%") == 0) {
      parsedInstruction.options =
          (PrismInstructionOptions){.RangeOptions =
#ifdef __cplusplus
                                        PrismInstructionOptions::
#endif
                                            PRISM_OPTION_RANGE_RELATIVE_END};
    } else if (strcmp(token, "#") == 0) {
      parsedInstruction.options =
          (PrismInstructionOptions){.RangeOptions =
#ifdef __cplusplus
                                        PrismInstructionOptions::
#endif
                                            PRISM_OPTION_RANGE_ABSOLUTE_START};
    } else if (strcmp(token, "##") == 0) {
      parsedInstruction.options =
          (PrismInstructionOptions){.RangeOptions =
#ifdef __cplusplus
                                        PrismInstructionOptions::
#endif
                                            PRISM_OPTION_RANGE_ABSOLUTE_END};
    } else {
      return (PrismInstruction){.instruction = PRISM_EXCEPTION,
                                .options = {.NoOptions = {}}};
    }

    // Next token is value
    token = strtok(NULL, " ");
    parsedInstruction.value = (uint8_t)strtol(token, NULL, 16);
  }

  /* Fill a range with a color */
  else if (strcmp(token, "FILL") == 0) {

    parsedInstruction.instruction = PRISM_INSTRUCTION_FILL;
    token = strtok(NULL, " ");

    switch (token[0]) {
    case 'R':
      parsedInstruction.options =
          (PrismInstructionOptions){.ColorOptions =
#ifdef __cplusplus
                                        PrismInstructionOptions::
#endif
                                            PRISM_OPTION_COLOR_RED};
      break;
    case 'G':
      parsedInstruction.options =
          (PrismInstructionOptions){.ColorOptions =
#ifdef __cplusplus
                                        PrismInstructionOptions::
#endif
                                            PRISM_OPTION_COLOR_GREEN};
      break;
    case 'B':
      parsedInstruction.options =
          (PrismInstructionOptions){.ColorOptions =
#ifdef __cplusplus
                                        PrismInstructionOptions::
#endif
                                            PRISM_OPTION_COLOR_BLUE};
      break;

    default:
      return (PrismInstruction){.instruction = PRISM_EXCEPTION,
                                .options = {.NoOptions = {}}};
    }

    // Next token is value
    token = strtok(NULL, " ");
    parsedInstruction.value = (uint8_t)strtol(token, NULL, 16);
  }

  /* Set the selected led with a color */
  else if (strcmp(token, "SET") == 0) {

    parsedInstruction.instruction = PRISM_INSTRUCTION_SET;
    token = strtok(NULL, " ");

    switch (token[0]) {
    case 'R':
      parsedInstruction.options =
          (PrismInstructionOptions){.ColorOptions =
#ifdef __cplusplus
                                        PrismInstructionOptions::
#endif
                                            PRISM_OPTION_COLOR_RED};
      break;
    case 'G':
      parsedInstruction.options =
          (PrismInstructionOptions){.ColorOptions =
#ifdef __cplusplus
                                        PrismInstructionOptions::
#endif
                                            PRISM_OPTION_COLOR_GREEN};
      break;
    case 'B':
      parsedInstruction.options =
          (PrismInstructionOptions){.ColorOptions =
#ifdef __cplusplus
                                        PrismInstructionOptions::
#endif
                                            PRISM_OPTION_COLOR_BLUE};
      break;

    default:
      return (PrismInstruction){.instruction = PRISM_EXCEPTION,
                                .options = {.NoOptions = {}}};
    }

    // Next token is value
    token = strtok(NULL, " ");
    parsedInstruction.value = (uint8_t)strtol(token, NULL, 16);
  }

  /* Blur effect */
  else if (strcmp(token, "EBLR") == 0) {
    parsedInstruction.instruction = PRISM_INSTRUCTION_BLUR;
    token = strtok(NULL, " ");

    if (strcmp(token, "ALL") == 0) {
      parsedInstruction.options =
          (PrismInstructionOptions){.EffectOptions =
#ifdef __cplusplus
                                        PrismInstructionOptions::
#endif
                                            PRISM_OPTION_APPLY_ALL};
    } else if (strcmp(token, "RAN") == 0) {
      parsedInstruction.options =
          (PrismInstructionOptions){.EffectOptions =
#ifdef __cplusplus
                                        PrismInstructionOptions::
#endif
                                            PRISM_OPTION_APPLY_RANGE};
    } else {
      return (PrismInstruction){.instruction = PRISM_EXCEPTION,
                                .options = {.NoOptions = {}}};
      ; //  Ill formed command
    }

    // Next token is value
    token = strtok(NULL, " ");
    parsedInstruction.value = (uint8_t)strtol(token, NULL, 16);
  }

  /* Sleep for specified amount of time */
  else if (strcmp(token, "SLP") == 0) {
    parsedInstruction.instruction = PRISM_INSTRUCTION_SLEEP;
    token = strtok(NULL, " ");

    if (strcmp(token, "US") == 0) {
      parsedInstruction.options = (PrismInstructionOptions){
          .TimeOptions =
#ifdef __cplusplus
              PrismInstructionOptions::
#endif
                  PRISM_OPTION_TIME_US,
      };
    } else if (strcmp(token, "MS") == 0) {
      parsedInstruction.options = (PrismInstructionOptions){
          .TimeOptions =
#ifdef __cplusplus
              PrismInstructionOptions::
#endif
                  PRISM_OPTION_TIME_MS,
      };
    } else if (strcmp(token, "SEC") == 0) {
      parsedInstruction.options =
          (PrismInstructionOptions){.TimeOptions =
#ifdef __cplusplus
                                        PrismInstructionOptions::
#endif
                                            PRISM_OPTION_TIME_SEC};
    } else if (strcmp(token, "MIN") == 0) {
      parsedInstruction.options = (PrismInstructionOptions){
          .TimeOptions =
#ifdef __cplusplus
              PrismInstructionOptions::
#endif
                  PRISM_OPTION_TIME_MIN,
      };
    } else {
      return (PrismInstruction){.instruction = PRISM_EXCEPTION,
                                .options = {.NoOptions = {}}};
    }

    // Next token is value
    token = strtok(NULL, " ");
    parsedInstruction.value = (uint8_t)strtol(token, NULL, 16);
  }

  /* Invalid instruction */
  else {
    return (PrismInstruction){.instruction = PRISM_EXCEPTION,
                              .options = {.NoOptions = {}}};
  }

  // Return the parsed instruction
  return parsedInstruction;
}

/**
 * @brief Parse instruction into a 16-bit binary format (Little Endian)
 *
 * @param instruction Instruction to be parsed
 * @return uint16_t 16 bit result
 */
static inline uint16_t
Newton_WriteInstructionToU16(const PrismInstruction instruction) {
  return *((uint16_t *)((_NewtonRawInstruction *)&instruction));
}

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif
