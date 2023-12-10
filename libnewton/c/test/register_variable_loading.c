
#include "newton/core/instruction.h"
#include "newton/core/parser.h"
#include "newton/defs.h"
#include <newton/core/registers.h>
#include <newton/core/variables.h>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CONFIG_BUFFER_SIZE 30

int main(int argc, char **argv) {
  /* Check for arguments */
  assert(argc == 2 && "Missing binary file params");

  /* Initialized the registers */
  NewtonVariables variables = Newton_CreateVariables(CONFIG_BUFFER_SIZE);
  NewtonRegisters registers = Newton_InitRegisters(variables);

  /* Read the file */
  FILE *file = fopen(argv[1], "rb");
  if (file == nullptr) {
    perror("Failed to open file");
    return EXIT_FAILURE;
  }

  // Create the memory layout
  struct {
    struct {
      uint8_t R;
      uint8_t G;
      uint8_t B;
    } Buffer[CONFIG_BUFFER_SIZE];
    uint8_t CurrentSelection;
  } InterpreterMemory = {.CurrentSelection = 0x0};

  memset(InterpreterMemory.Buffer, 0, sizeof(InterpreterMemory.Buffer));

  // Check X is version
  assert(registers.X == LIBNEWTON_VERSION && "X is set to LIBNEWTON_VERSION");
  // Check Y is buffer size
  assert(registers.Y == sizeof(InterpreterMemory.Buffer) /
                            sizeof(InterpreterMemory.Buffer[0]) &&
         "Y is set to Buffer Size");

  while (feof(file) == 0) {
    // Read bytes
    uint16_t rawBytes = 0x0000;
    fread((void *)&rawBytes, sizeof(uint16_t), 1, file);

    if (rawBytes == 0x00) {
      break;
    }

    { /* swap bytes for Little Endian */
      uint8_t _auxBytes = ((rawBytes & 0xFF00) >> 8);
      rawBytes <<= 8;
      rawBytes |= (_auxBytes);
    }

    fprintf(stdout, "> 0x%04X\n", rawBytes);
    fflush(stdout);
    PrismInstruction instruction = Newton_ParseInstructionU16(rawBytes);

    switch (instruction.instruction) {

    case PRISM_INSTRUCTION_SELECT:
      switch (instruction.options.SelectOptions) {
      case PRISM_OPTION_SELECT_ABSOLUTE:
        InterpreterMemory.CurrentSelection = instruction.value;
        break;
      default:
        fprintf(stderr, "Relative indexing not supported by the test");
        abort();
      }
      break;

    case PRISM_INSTRUCTION_SET:
      switch (instruction.options.ColorOptions) {
      case PRISM_OPTION_COLOR_RED:
        InterpreterMemory.Buffer[InterpreterMemory.CurrentSelection].R =
            instruction.value;
        break;

      case PRISM_OPTION_COLOR_GREEN:
        InterpreterMemory.Buffer[InterpreterMemory.CurrentSelection].G =
            instruction.value;
        break;

      case PRISM_OPTION_COLOR_BLUE:
        InterpreterMemory.Buffer[InterpreterMemory.CurrentSelection].B =
            instruction.value;
        break;
      }
      break;

    case PRISM_INSTRUCTION_LOADX:
      switch (instruction.options.LoadOptions) {
      case PRISM_OPTION_LOAD_VARIABLE:
        switch (instruction.value) {
        case NEWTON_VARIABLES_S:
          registers.X = variables.BufferSize;
          break;
        case NEWTON_VARIABLES_V:
          registers.X = variables.NewtonVersion;
          break;
        case INVALID_VARIABLE:
          [[fallthrough]];
        default:
          fprintf(stderr, "Invalid variable index `%d`", instruction.value);
          abort();
        }
        break;
      case PRISM_OPTION_LOAD_R:
        registers.X = InterpreterMemory.Buffer[instruction.value].R;
        break;
      case PRISM_OPTION_LOAD_G:
        registers.X = InterpreterMemory.Buffer[instruction.value].G;
        break;
      case PRISM_OPTION_LOAD_B:
        registers.X = InterpreterMemory.Buffer[instruction.value].B;
        break;
      }
      break;

    case PRISM_INSTRUCTION_LOADY:
      switch (instruction.options.LoadOptions) {
      case PRISM_OPTION_LOAD_VARIABLE:
        switch (instruction.value) {
        case NEWTON_VARIABLES_S:
          registers.Y = variables.BufferSize;
          break;
        case NEWTON_VARIABLES_V:
          registers.Y = variables.NewtonVersion;
          break;
        case INVALID_VARIABLE:
          [[fallthrough]];
        default:
          fprintf(stderr, "Invalid variable index `%d`", instruction.value);
          abort();
        }
        break;
      case PRISM_OPTION_LOAD_R:
        registers.Y = InterpreterMemory.Buffer[instruction.value].R;
        break;
      case PRISM_OPTION_LOAD_G:
        registers.Y = InterpreterMemory.Buffer[instruction.value].G;
        break;
      case PRISM_OPTION_LOAD_B:
        registers.Y = InterpreterMemory.Buffer[instruction.value].B;
        break;
      }
      break;

    default:
      fprintf(stderr, "Unsupported instruction for this test");
      abort();
    }
  }

  // Check Y is version
  assert(registers.Y == LIBNEWTON_VERSION && "Y is set to V");
  // Check X is buffer size
  assert(registers.X == sizeof(InterpreterMemory.Buffer) /
                            sizeof(InterpreterMemory.Buffer[0]) &&
         "X is set to S");

  fclose(file);
  return EXIT_SUCCESS;
}
