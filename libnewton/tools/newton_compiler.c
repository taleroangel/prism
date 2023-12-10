#define _POSIX_C_SOURCE 200809L

#include <newton.h>

#include <assert.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Show invalid arguments error and stop execution
 *
 */
void ArgumentError() {
  fprintf(stderr, "Not enough arguments:\
				\n\t[-i <InputFile> -o <OutputFile>]\n");
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

  char *InputFilename = nullptr;
  char *OutputFilename = nullptr;

  { /* _optionSelection block*/
    int _optionSelection;

    while ((_optionSelection = getopt(argc, argv, "i:o:s:")) > 0) {
      switch (_optionSelection) {
        /* Get input filename*/
      case 'i':
        InputFilename = optarg;
        break;

        /* Get output filename*/
      case 'o':
        OutputFilename = optarg;
        break;

      case '?':
        [[fallthrough]];

      default:
        ArgumentError();
      }
    }
  }

  /* Validate parameters */
  if (InputFilename == nullptr || OutputFilename == nullptr) {
    ArgumentError();
  } else {
    printf("Parsing and compiling: `%s`\n", InputFilename);
  }

  /* Open files */

  FILE *InputFile = fopen(InputFilename, "r");
  if (InputFile == nullptr) {
    perror("Failed to open input file");
    return EXIT_FAILURE;
  }

  FILE *OutputFile = fopen(OutputFilename, "wb");
  if (OutputFile == nullptr) {
    perror("Failed to create output file");
    fclose(InputFile);
    return EXIT_FAILURE;
  }

  { /* _readerBufferLine & _readBytes & _lenRead lifetime block */
    ssize_t _readBytes = 0;
    size_t _lenRead = 0;

    char *_readerBufferLine = nullptr;

    /* Read file line by line and compile */
    while ((_readBytes = getline(&_readerBufferLine, &_lenRead, InputFile)
            /*//! getline allocates memory (man 3 getline) */
            ) > 0) {

      /* Remove newline */
      { /* _newlineCharacter lifetime block */
        char *_newlineCharacter = strchr(_readerBufferLine, '\n');
        if (_newlineCharacter != nullptr) {
          *_newlineCharacter = '\0';
        }
      }

      /* Instruction parsing */
      PrismInstruction ReaderInstruction =
          Newton_ParseInstructionLiteral(_readerBufferLine);

      /* Check instruction NOP */
      switch (ReaderInstruction.instruction) {
      case PRISM_IGNORE_INSTRUCTION:
        continue;

      case PRISM_EXCEPTION:
        fprintf(stderr, "Exception thrown during instruction decoding:\
				\n(Stacktrace) -> \
					\n[[ ACTUAL ]]\
					\n\tLiteral: `%s`\
					\n\tBinary: `0x%02X`\
					\n[[ RESULT ]]\
					\n\tInstruction: `%d`\
					\n\tOptions: `%d`\
					\n\tValue: `0x%02X`\n",
                _readerBufferLine, ReaderInstruction.instruction,
                Newton_WriteInstructionToU16(ReaderInstruction),
                *(uint8_t *)(&ReaderInstruction.options),
                ReaderInstruction.value);
        return EXIT_FAILURE;

        /* Any other instruction is parsed*/
      default:
        break;
      }

      /* Instruction to binary */
      uint16_t InstructionBytes =
          Newton_WriteInstructionToU16(ReaderInstruction);

      PrismInstruction coherentInstruction =
          Newton_ParseInstructionU16(InstructionBytes);

      // Check that instruction is coherent
      assert(*(uint16_t *)(&coherentInstruction) ==
             *(uint16_t *)(&ReaderInstruction));

      /* Write the instruction (as BigEndian) */
      fwrite((void *)((uint8_t *)&InstructionBytes + 1), sizeof(uint8_t), 1,
             OutputFile);
      fwrite((void *)((uint8_t *)&InstructionBytes + 0), sizeof(uint8_t), 1,
             OutputFile);
			 
      fflush(OutputFile);
    }

    /* Deallocate memory */
    if (_readerBufferLine)
      free(_readerBufferLine);
  }

  /* Close files */

  fclose(InputFile);
  fclose(OutputFile);

  /* Finish message */
  printf("Successfully finished compilation:\
	\n\tOutput stored in `%s`\n",
         OutputFilename);

  return EXIT_SUCCESS;
}
