#include "newton/registers.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <newton.h>

int main(int argc, char **argv) {
  // Check arguments
  assert(argc == 3 && "Missing instructions file argument");

  // Open the instruction file
  FILE *instructionsFile = fopen(argv[1], "r");
  if (instructionsFile == nullptr) {
    perror("Failed to open instruction file");
    assert(instructionsFile != nullptr);
  }

  FILE *expectedFile = fopen(argv[2], "rb");
  if (expectedFile == nullptr) {
    perror("Failed to open binary file");
    assert(instructionsFile != nullptr);
  }

  // Create registers
  NewtonRegisters registers;
  memset((void *)&registers, 0, sizeof(NewtonRegisters));

  // Create buffer
  char readingBuffer[256];
  memset(readingBuffer, '\0', sizeof(readingBuffer));

  // Read line by line
  while (fgets(readingBuffer, sizeof(readingBuffer), instructionsFile) !=
         nullptr) {
    // Check newline
    char *newLineCharacter = strchr(readingBuffer, '\n');
    if (newLineCharacter != nullptr) {
      *newLineCharacter = '\0';
    }

    // Parse instruction
    fflush(stdout);

    char parseLiteral[255];
    strcpy(parseLiteral, readingBuffer);

    PrismInstruction bufferInstruction =
        Newton_ParseInstructionLiteral(parseLiteral);

    // Skip NOP operation
    if (bufferInstruction.instruction == PRISM_IGNORE_INSTRUCTION)
      continue;

    // Read from the binary file
    uint16_t expectedBinary = 0x00;
    fread(&expectedBinary, sizeof(uint16_t), 1, expectedFile);

    // Swap endianness
    { /* _auxBinary lifetime block */
      uint16_t _auxBinary = expectedBinary;
      ((uint8_t *)(&expectedBinary))[0] = ((uint8_t *)(&_auxBinary))[1];
      ((uint8_t *)(&expectedBinary))[1] = ((uint8_t *)(&_auxBinary))[0];
    }

    fprintf(stdout, "> 0x%04X\t", expectedBinary);
    fprintf(stdout, ":: %s\n", readingBuffer);
    fflush(stdout);

    // Buffer to binary and assert
    uint16_t bufferBinary = Newton_WriteInstructionToU16(bufferInstruction);
    assert(expectedBinary == bufferBinary);
  }

  // Close the file
  fclose(instructionsFile);
  fclose(expectedFile);
  return 0;
}
