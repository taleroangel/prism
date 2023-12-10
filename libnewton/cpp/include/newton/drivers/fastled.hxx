#ifndef __LIBNEWTON_DRIVERS_FASTLED_HXX__
#define __LIBNEWTON_DRIVERS_FASTLED_HXX__

#ifndef __cplusplus
#error "This library is not C compatible, use clibnewton instead"
#endif

#include <FastLED.h>

#include <newton/errors.hxx>
#include <newton/core/instruction.hxx>
#include <newton/core/registers.hxx>
#include <newton/core/variables.hxx>

#include <cstdint>
#include <variant>

namespace newton::drivers::fastled
{
	template <uint8_t BufferSize>
	class Interpreter
	{
	public:
		CRGB PublicBuffer[BufferSize];
		newton::core::Registers InterpreterRegisters;
		newton::core::Variables InterpreterVariables;

	private:
		CRGB PrivateBuffer[BufferSize];
		size_t CurrentSelection = 0;
		size_t RanStart = 0;
		size_t RanEnd = 0;

	public:
		inline Interpreter() : InterpreterVariables(BufferSize), InterpreterRegisters(InterpreterVariables)
		{
			this->InterpreterRegisters = {InterpreterVariables};
			memset(PublicBuffer, 0, sizeof(PublicBuffer));
			memset(PrivateBuffer, 0, sizeof(PrivateBuffer));
		}

		/**
		 * Execute and instruction
		 * @return Amount of time to delay
		 */
		inline std::variant<newton::Error, uint32_t> Instruction(const newton::core::Instruction &instruction)
		{

			switch (instruction.instruction)
			{
				// Update
			case newton::core::InstructionSet::UPDATE:
				memcpy(PublicBuffer, PrivateBuffer, sizeof(PrivateBuffer));
				break;

				// Clear
			case newton::core::InstructionSet::CLEAR:
				memset(PrivateBuffer, 0, sizeof(PrivateBuffer));
				break;

				// Select
			case newton::core::InstructionSet::SELECT:
				switch (instruction.options.SelectOptions)
				{
				case newton::core::InstructionOptions::OPTION_SELECT_ABSOLUTE:
					CurrentSelection = instruction.value;
					break;

				case newton::core::InstructionOptions::OPTION_SELECT_RELATIVE:
					CurrentSelection = map8(instruction.value, 0, BufferSize - 1);
					break;
				}
				break;

				// Range
			case newton::core::InstructionSet::RANGE:
				switch (instruction.options.RangeOptions)
				{
				case newton::core::InstructionOptions::OPTION_RANGE_ABSOLUTE_START:
					RanStart = instruction.value;
					break;

				case newton::core::InstructionOptions::OPTION_RANGE_ABSOLUTE_END:
					RanEnd = instruction.value;
					break;

				case newton::core::InstructionOptions::OPTION_RANGE_RELATIVE_START:
					RanStart = map8(instruction.value, 0, BufferSize - 1);
					break;

				case newton::core::InstructionOptions::OPTION_RANGE_RELATIVE_END:
					RanEnd = map8(instruction.value, 0, BufferSize - 1);
					break;
				}
				break;

			case newton::core::InstructionSet::FILL:
				for (size_t ii = RanStart; ii <= RanEnd; ii++)
					switch (instruction.options.ColorOptions)
					{
					case newton::core::InstructionOptions::OPTION_COLOR_RED:
						PrivateBuffer[ii].red = instruction.value;
						break;

					case newton::core::InstructionOptions::OPTION_COLOR_GREEN:
						PrivateBuffer[ii].green = instruction.value;
						break;

					case newton::core::InstructionOptions::OPTION_COLOR_BLUE:
						PrivateBuffer[ii].blue = instruction.value;
						break;
					}
				break;

			case newton::core::InstructionSet::SET:
				switch (instruction.options.ColorOptions)
				{
				case newton::core::InstructionOptions::OPTION_COLOR_RED:
					PrivateBuffer[CurrentSelection].red = instruction.value;
					break;

				case newton::core::InstructionOptions::OPTION_COLOR_GREEN:
					PrivateBuffer[CurrentSelection].green = instruction.value;
					break;

				case newton::core::InstructionOptions::OPTION_COLOR_BLUE:
					PrivateBuffer[CurrentSelection].blue = instruction.value;
					break;
				}
				break;

			case newton::core::InstructionSet::BLUR:
				switch (instruction.options.EffectOptions)
				{
				case newton::core::InstructionOptions::OPTION_APPLY_ALL:
					blur1d(PrivateBuffer, BufferSize, instruction.value);
					break;

				case newton::core::InstructionOptions::OPTION_APPLY_RANGE:
					blur1d(PrivateBuffer + RanStart, (RanEnd - RanStart), instruction.value);
					break;
				}
				break;

			case newton::core::InstructionSet::SLEEP:
				switch (instruction.options.TimeOptions)
				{
				case newton::core::InstructionOptions::OPTION_TIME_MS:
					return (unsigned long)instruction.value;

				case newton::core::InstructionOptions::OPTION_TIME_SEC:
					return ((unsigned long)instruction.value * 1000UL);

				case newton::core::InstructionOptions::OPTION_TIME_MIN:
					return (unsigned long)(instruction.value * 60000UL);
				}
				break;

			case newton::core::InstructionSet::LOADY:
				switch (instruction.options.LoadOptions)
				{
				case newton::core::InstructionOptions::OPTION_LOAD_VARIABLE:
					switch (instruction.value)
					{
					case newton::core::VariablesCodes::NEWTON_VARIABLES_S:
						InterpreterRegisters.Y = InterpreterVariables.BufferSize;
						break;

					case newton::core::VariablesCodes::NEWTON_VARIABLES_V:
						InterpreterRegisters.Y = InterpreterVariables.NewtonVersion;
						break;

					case newton::core::VariablesCodes::INVALID_VARIABLE:
						[[fallthrough]];
					default:
						return newton::Error::INVALID_VARIABLE_CODE;
					}
					break;

				case newton::core::InstructionOptions::OPTION_LOAD_R:
					InterpreterRegisters.Y = PrivateBuffer[static_cast<size_t>(instruction.value)].red;
					break;

				case newton::core::InstructionOptions::OPTION_LOAD_G:
					InterpreterRegisters.Y = PrivateBuffer[static_cast<size_t>(instruction.value)].green;
					break;

				case newton::core::InstructionOptions::OPTION_LOAD_B:
					InterpreterRegisters.Y = PrivateBuffer[static_cast<size_t>(instruction.value)].blue;
					break;
				}
				break;

			case newton::core::InstructionSet::LOADX:
				switch (instruction.options.LoadOptions)
				{
				case newton::core::InstructionOptions::OPTION_LOAD_VARIABLE:
					switch (instruction.value)
					{
					case newton::core::VariablesCodes::NEWTON_VARIABLES_S:
						InterpreterRegisters.X = InterpreterVariables.BufferSize;
						break;

					case newton::core::VariablesCodes::NEWTON_VARIABLES_V:
						InterpreterRegisters.X = InterpreterVariables.NewtonVersion;
						break;

					case newton::core::VariablesCodes::INVALID_VARIABLE:
						[[fallthrough]];
					default:
						return newton::Error::INVALID_VARIABLE_CODE;
					}
					break;

				case newton::core::InstructionOptions::OPTION_LOAD_R:
					InterpreterRegisters.X = PrivateBuffer[static_cast<size_t>(instruction.value)].red;
					break;

				case newton::core::InstructionOptions::OPTION_LOAD_G:
					InterpreterRegisters.X = PrivateBuffer[static_cast<size_t>(instruction.value)].green;
					break;

				case newton::core::InstructionOptions::OPTION_LOAD_B:
					InterpreterRegisters.X = PrivateBuffer[static_cast<size_t>(instruction.value)].blue;
					break;
				}
				break;

			case newton::core::InstructionSet::IGNORE_INSTRUCTION:
				break;

			case newton::core::InstructionSet::EXCEPTION:
				return newton::Error::EXCEPTION_INSTRUCTION;
			}

			// No delay
			return 0UL;
		}
	};

}

#endif