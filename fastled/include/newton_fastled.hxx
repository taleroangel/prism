#ifndef __LIBNEWTON_FASTLED__
#define __LIBNEWTON_FASTLED__

#include <FastLED.h>
#include <cstdint>
#include <newton.h>

#if !defined(ARDUINO) && !defined(ESP8266) && !defined(ESP32) &&               \
    !defined(IGNORE_ARDUINO_WARNING)
#warning                                                                       \
    "Neither ARDUINO nor ESP8266 are defined, FastLED module might not work. To disable this warning define IGNORE_ARDUINO_WARNING)"
#endif

template <uint8_t MAX_LEDS> class NewtonFastLEDInterpreter {
public:
  CRGB LedBuffer[MAX_LEDS];
  NewtonRegisters Registers = {
      .X = MAX_LEDS,
      .Y = 0x00,
  };

private:
  CRGB Buffer[MAX_LEDS];
  size_t CurrentSelection = 0;
  size_t RanStart = 0;
  size_t RanEnd = 0;

public:
  NewtonFastLEDInterpreter() = default;

  inline void Instruction(const PrismInstruction &instruction) {
    switch (instruction.instruction) {
      // Update
    case PrismInstructionSet::PRISM_INSTRUCTION_UPDATE:
      memcpy(LedBuffer, Buffer, sizeof(Buffer));
      break;

      // Clear
    case PrismInstructionSet::PRISM_INSTRUCTION_CLEAR:
      memset(Buffer, 0, sizeof(Buffer));
      break;

      // Select
    case PrismInstructionSet::PRISM_INSTRUCTION_SELECT:
      switch (instruction.options.SelectOptions) {
      case PrismInstructionOptions::PRISM_OPTION_SELECT_ABSOLUTE:
        CurrentSelection = instruction.value;
        break;

      case PrismInstructionOptions::PRISM_OPTION_SELECT_RELATIVE:
        CurrentSelection = map8(instruction.value, 0, MAX_LEDS - 1);
        break;
      }
      break;

      // Range
    case PrismInstructionSet::PRISM_INSTRUCTION_RANGE:
      switch (instruction.options.RangeOptions) {
      case PrismInstructionOptions::PRISM_OPTION_RANGE_ABSOLUTE_START:
        RanStart = instruction.value;
        break;

      case PrismInstructionOptions::PRISM_OPTION_RANGE_ABSOLUTE_END:
        RanEnd = instruction.value;
        break;

      case PrismInstructionOptions::PRISM_OPTION_RANGE_RELATIVE_START:
        RanStart = map8(instruction.value, 0, MAX_LEDS - 1);
        break;

      case PrismInstructionOptions::PRISM_OPTION_RANGE_RELATIVE_END:
        RanEnd = map8(instruction.value, 0, MAX_LEDS - 1);
        break;
      }
      break;

    case PrismInstructionSet::PRISM_INSTRUCTION_FILL:
      for (size_t ii = RanStart; ii <= RanEnd; ii++)
        switch (instruction.options.ColorOptions) {
        case PrismInstructionOptions::PRISM_OPTION_COLOR_RED:
          Buffer[ii].red = instruction.value;
          break;

        case PrismInstructionOptions::PRISM_OPTION_COLOR_GREEN:
          Buffer[ii].green = instruction.value;
          break;

        case PrismInstructionOptions::PRISM_OPTION_COLOR_BLUE:
          Buffer[ii].blue = instruction.value;
          break;
        }
      break;

    case PrismInstructionSet::PRISM_INSTRUCTION_SET:
      switch (instruction.options.ColorOptions) {
      case PrismInstructionOptions::PRISM_OPTION_COLOR_RED:
        Buffer[CurrentSelection].red = instruction.value;
        break;

      case PrismInstructionOptions::PRISM_OPTION_COLOR_GREEN:
        Buffer[CurrentSelection].green = instruction.value;
        break;

      case PrismInstructionOptions::PRISM_OPTION_COLOR_BLUE:
        Buffer[CurrentSelection].blue = instruction.value;
        break;
      }
      break;

    case PrismInstructionSet::PRISM_INSTRUCTION_BLUR:
      switch (instruction.options.EffectOptions) {
      case PrismInstructionOptions::PRISM_OPTION_APPLY_ALL:
        blur1d(Buffer, MAX_LEDS, instruction.value);
        break;

      case PrismInstructionOptions::PRISM_OPTION_APPLY_RANGE:
        blur1d(Buffer + RanStart, (RanEnd - RanStart), instruction.value);
        break;
      }
      break;

    case PrismInstructionSet::PRISM_INSTRUCTION_SLEEP:
      switch (instruction.options.TimeOptions) {
      case PrismInstructionOptions::PRISM_OPTION_TIME_US:
        delayMicroseconds((unsigned int)instruction.value);
        break;

      case PrismInstructionOptions::PRISM_OPTION_TIME_MS:
        delay((unsigned long)instruction.value);
        break;

      case PrismInstructionOptions::PRISM_OPTION_TIME_SEC:
        delay(((unsigned long)instruction.value * 1000UL));
        break;

      case PrismInstructionOptions::PRISM_OPTION_TIME_MIN:
        delay((unsigned long)(instruction.value * 60000UL));
        break;
      }
      break;

    case PrismInstructionSet::PRISM_INSTRUCTION_LOADY:
      switch (instruction.options.LoadOptions) {
      case PrismInstructionOptions::PRISM_OPTION_LOAD_SIZE:
        Registers.Y = MAX_LEDS;
        break;

      case PrismInstructionOptions::PRISM_OPTION_LOAD_R:
        Registers.Y = Buffer[static_cast<size_t>(instruction.value)].red;
        break;

      case PrismInstructionOptions::PRISM_OPTION_LOAD_G:
        Registers.Y = Buffer[static_cast<size_t>(instruction.value)].green;
        break;

      case PrismInstructionOptions::PRISM_OPTION_LOAD_B:
        Registers.Y = Buffer[static_cast<size_t>(instruction.value)].blue;
        break;
      }
      break;

    case PrismInstructionSet::PRISM_INSTRUCTION_LOADX:
      switch (instruction.options.LoadOptions) {
      case PrismInstructionOptions::PRISM_OPTION_LOAD_SIZE:
        Registers.X = MAX_LEDS;
        break;
        [[fallthrough]];

      case PrismInstructionOptions::PRISM_OPTION_LOAD_R:
        Registers.X = Buffer[static_cast<size_t>(instruction.value)].red;
        break;

      case PrismInstructionOptions::PRISM_OPTION_LOAD_G:
        Registers.X = Buffer[static_cast<size_t>(instruction.value)].green;
        break;

      case PrismInstructionOptions::PRISM_OPTION_LOAD_B:
        Registers.X = Buffer[static_cast<size_t>(instruction.value)].blue;
        break;
      }
      break;

    case PrismInstructionSet::PRISM_IGNORE_INSTRUCTION:
      break;

    /* !! Abort execution */
    case PrismInstructionSet::PRISM_EXCEPTION:
      abort();
    }
  }
};

#endif