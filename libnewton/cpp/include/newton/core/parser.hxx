#ifndef __LIBNEWTON_CORE_PARSER_HXX__
#define __LIBNEWTON_CORE_PARSER_HXX__

#ifndef __cplusplus
#error "This library is not C compatible, use clibnewton instead"
#endif

#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "instruction.hxx"

namespace newton::core
{

	/**
	 * @brief Parse a 16 byte integer (Little Endian) into a
	 * \struct{Instruction}
	 *
	 * @param byte Raw 16 bytes in LittleEndian format
	 * @return Instruction Parsed instruction
	 */
	static inline Instruction ParseInstructionU16(uint16_t byte)
	{

		_RawInstruction rawInstruction = {
			.instruction = static_cast<uint8_t>(((byte & 0xFC00) >> 10)),
			.options = static_cast<uint8_t>(((byte & 0x0300) >> 8)),
			.value = static_cast<uint8_t>((byte & 0x00FF)),
		};

		Instruction parsedInstruction = {
			.instruction = (InstructionSet)rawInstruction.instruction,
			.options = {},
			.value = rawInstruction.value,
		};

		uint8_t optionsValue = rawInstruction.options;
		memcpy(&parsedInstruction.options, &optionsValue, sizeof(uint8_t));

		return parsedInstruction;
	}

	/**
	 * @brief Parse instruction into a 16-bit binary format (Little Endian)
	 *
	 * @param instruction Instruction to be parsed
	 * @return uint16_t 16 bit result
	 */
	static inline uint16_t WriteInstructionToU16(const Instruction instruction)
	{

		const _RawInstruction rawInstruction = {
			.instruction = instruction.instruction,
			.options = *(uint8_t *)(&instruction.options),
			.value = instruction.value};

		uint16_t instructionBytes = 0x0000UL;

		instructionBytes |= ((uint16_t)rawInstruction.options << 8);
		instructionBytes |= (((uint16_t)rawInstruction.instruction << 10) & 0xFF00);
		instructionBytes |= rawInstruction.value;

		// Parse value into bits
		return instructionBytes;
	}

	/**
	 * @brief Parse an instruction literal into a \struct{Instruction}
	 *
	 * @param instruction Literal with the instruction without \\n character
	 * @return Instruction Built Parsed instruction
	 */
	static inline Instruction ParseInstructionLiteral(const char *instruction)
	{

		// If instruction pointer is null return exception
		if (instruction == NULL)
		{
			return {
				.instruction = newton::core::InstructionSet::EXCEPTION,
				.options = {.NoOptions = {}},
			};
		}

		// Contains comment or empty line (Ignore instruction)
		if ((strncmp(instruction, "--", 2) == 0) || (strlen(instruction) <= 2))
		{
			return {
				.instruction = newton::core::InstructionSet::IGNORE_INSTRUCTION,
				.options = {.NoOptions = {}},
			};
		}

		// Generate the instruction
		Instruction parsedInstruction = {
			.instruction = newton::core::InstructionSet::IGNORE_INSTRUCTION,
			.options = {.NoOptions = {}},
		};

		// Tokenize instructions
		char *token = strtok((char *)instruction, " ");

		/* Ignore instruction */
		if (strcmp(token, "NOP") == 0)
		{
			return parsedInstruction; /* Do nothing */
		}

		/* Update the buffer */
		else if (strcmp(token, "UPDT") == 0)
		{
			parsedInstruction.instruction = newton::core::InstructionSet::UPDATE;
			return parsedInstruction;
		}

		/* Clear the buffer */
		else if (strcmp(token, "CLR") == 0)
		{
			parsedInstruction.instruction = newton::core::InstructionSet::CLEAR;
			return parsedInstruction;
		}

		else if (strcmp(token, "LDX") == 0 || strcmp(token, "LDY") == 0)
		{

			// Set the instruction
			parsedInstruction.instruction = strcmp(token, "LDX") == 0 ? LOADX : LOADY;
			// Next token is mode
			token = strtok(NULL, " ");

			switch (token[0])
			{
			case '$':
				parsedInstruction.options.LoadOptions = InstructionOptions::OPTION_LOAD_VARIABLE;
				break;

			case 'R':
				parsedInstruction.options.LoadOptions = InstructionOptions::OPTION_LOAD_R;
				break;

			case 'G':
				parsedInstruction.options.LoadOptions = InstructionOptions::OPTION_LOAD_G;
				break;

			case 'B':
				parsedInstruction.options.LoadOptions = InstructionOptions::OPTION_LOAD_B;
				break;

				// Invalid token detected
			default:
				return {.instruction = newton::core::InstructionSet::EXCEPTION};
			}
		}

		/* Select a value from buffer */
		else if (strcmp(token, "SEL") == 0)
		{

			parsedInstruction.instruction = newton::core::InstructionSet::SELECT;
			// Next token is mode
			token = strtok(NULL, " ");

			switch (token[0])
			{
				/* For relative selection */
			case '%':
				parsedInstruction.options.SelectOptions = InstructionOptions::OPTION_SELECT_RELATIVE;
				break;

				/* For absolute selection */
			case '#':
				parsedInstruction.options.SelectOptions = InstructionOptions::OPTION_SELECT_ABSOLUTE;
				break;

				// Invalid token detected
			default:
				return {.instruction = newton::core::InstructionSet::EXCEPTION};
			}
		}

		/* Select a range from buffer */
		else if (strcmp(token, "RAN") == 0)
		{

			parsedInstruction.instruction = newton::core::InstructionSet::RANGE;
			token = strtok(NULL, " ");

			if (strcmp(token, "%") == 0)
			{
				parsedInstruction.options.RangeOptions = InstructionOptions::OPTION_RANGE_RELATIVE_START;
			}
			else if (strcmp(token, "%%") == 0)
			{
				parsedInstruction.options.RangeOptions = InstructionOptions::OPTION_RANGE_RELATIVE_END;
			}
			else if (strcmp(token, "#") == 0)
			{
				parsedInstruction.options.RangeOptions = InstructionOptions::OPTION_RANGE_ABSOLUTE_START;
			}
			else if (strcmp(token, "##") == 0)
			{
				parsedInstruction.options.RangeOptions = InstructionOptions::OPTION_RANGE_ABSOLUTE_END;
			}
			else
			{

				// Invalid token detected
				return {.instruction = newton::core::InstructionSet::EXCEPTION};
			}
		}

		/* Fill a range with a color */
		else if (strcmp(token, "FILL") == 0)
		{

			parsedInstruction.instruction = newton::core::InstructionSet::FILL;
			token = strtok(NULL, " ");

			switch (token[0])
			{
			case 'R':
				parsedInstruction.options.ColorOptions = InstructionOptions::OPTION_COLOR_RED;
				break;
			case 'G':
				parsedInstruction.options.ColorOptions = InstructionOptions::OPTION_COLOR_GREEN;
				break;
			case 'B':
				parsedInstruction.options.ColorOptions = InstructionOptions::OPTION_COLOR_BLUE;
				break;

				// Invalid token detected
			default:
				return {.instruction = newton::core::InstructionSet::EXCEPTION};
			}
		}

		/* Set the selected led with a color */
		else if (strcmp(token, "SET") == 0)
		{

			parsedInstruction.instruction = newton::core::InstructionSet::SET;
			token = strtok(NULL, " ");

			switch (token[0])
			{
			case 'R':
				parsedInstruction.options.ColorOptions = InstructionOptions::OPTION_COLOR_RED;
				break;
			case 'G':
				parsedInstruction.options.ColorOptions = InstructionOptions::OPTION_COLOR_GREEN;
				break;
			case 'B':
				parsedInstruction.options.ColorOptions = InstructionOptions::OPTION_COLOR_BLUE;
				break;

				// Invalid token detected
			default:
				return {.instruction = newton::core::InstructionSet::EXCEPTION};
			}
		}

		/* Blur effect */
		else if (strcmp(token, "EBLR") == 0)
		{

			parsedInstruction.instruction = newton::core::InstructionSet::BLUR;
			token = strtok(NULL, " ");

			if (strcmp(token, "ALL") == 0)
			{
				parsedInstruction.options.EffectOptions = InstructionOptions::OPTION_APPLY_ALL;
			}
			else if (strcmp(token, "RAN") == 0)
			{
				parsedInstruction.options.EffectOptions = InstructionOptions::OPTION_APPLY_RANGE;
			}
			else
			{
				// Invalid token detected
				return {.instruction = newton::core::InstructionSet::EXCEPTION};
			}
		}

		/* Sleep for specified amount of time */
		else if (strcmp(token, "SLP") == 0)
		{

			parsedInstruction.instruction = newton::core::InstructionSet::SLEEP;
			token = strtok(NULL, " ");

			if (strcmp(token, "MS") == 0)
			{
				parsedInstruction.options.TimeOptions = InstructionOptions::OPTION_TIME_MS;
			}
			else if (strcmp(token, "SEC") == 0)
			{
				parsedInstruction.options.TimeOptions = InstructionOptions::OPTION_TIME_SEC;
			}
			else if (strcmp(token, "MIN") == 0)
			{
				parsedInstruction.options.TimeOptions = InstructionOptions::OPTION_TIME_MIN;
			}
			else
			{
				// Invalid token detected
				return {.instruction = newton::core::InstructionSet::EXCEPTION};
			}
		}

		/* Invalid instruction */
		else
		{
			// Invalid token detected
			return {
				.instruction = newton::core::InstructionSet::EXCEPTION,
				.options = {.NoOptions = {}},
			};
		}

		// Next token is value
		token = strtok(NULL, " ");
		parsedInstruction.value = static_cast<uint8_t>(std::strtol(token, NULL, 16));

		// Return the parsed instruction
		return parsedInstruction;
	}

} // namespace newton::core

#endif
