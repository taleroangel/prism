# Prism
Prism is a protocol for manipulating LED strips at runtime with a predefined set of instructions in a _Master - Slave_ model

Prism is meant to be extensible in order to allow multiple independent programs to communicate with LED strips

## Roles

![Roles Diagram](docs/roles.svg)

### Master

### Slave
A Prism slave is a device attached to a LED strip and interprets commands sent by a [master](#master) in order to trigger changes in color. Slaves will always have a [Newton Interpreter](#newton) which contains the inner [registers](#registers) and interprets commands via [libnewton](#libnewton).

- Slaves can have different LED drivers (FastLED driver is bundled `/fastled/include`)
- Slaves can communicate via different unique protocols as long as the [master](#master) supports them. [(List of standard communication protocols)](#standard-communication-protocols)
## Buffer
The buffer is the main data structure inside the [Prism Slave](#slave), it is an array of RGB values whose size is the same as the attached LED strip, each position of the array is 3 bytes long (one byte for each RGB value) and the contents are dumped to the LED strip with the `UPDT` instruction ([See Basic Instructions](#basic-instructions))

## Registers
The [Prism Slave](#slave) has two 1-byte registers that can be read and set from the master

| Register | Default Value                      |
| -------- | ---------------------------------- |
| X        | [S constant](#size-number-of-leds) |
| Y        | 0x00                               |

Either an RGB value from the [buffer](#buffer) or the [S constant](#size-number-of-leds) can be stored inside a register using `LDX` and `LDY` instructions. [See LoadInstructions](#load-instructions).

### Size (Number of LEDs)
The number of LEDs attached to the strip is stored in a constant called **S**, by default the [X register](#registers) is set on startup to **S**

**S** can be loaded into a register by the `LDX S` and `LDY S` instructions

## Instruction Set
Prism provides a basic set of instructions for manipulating the [buffer](#buffer) and the [slave's](#slave) behaviour.


Instructions are sent from [master](#master) to [slave](#slave) in a **16-bit Big Endian** frame, the first 6-bits correspond to the instruction, next 2-bits correspond to the options and the last 8-bits correspond to the value.

![Instruction memory layout](docs/instruction_layout.svg)

Instructions can also be compiled from a _Literal_ into a _Binary_ format with the _Newton Compiler_ `/libnewton/tools/newton_compiler.c`, a _Literal_ interpreter shell with the ESP8266 example can be found in `/fastled/examples/esp8266_shell`

Example of a Prism file with _Instruction Literals_:

```
-- Set all leds in strip to white
CLR
RAN % 00
RAN %% FF
FILL R FF
FILL G FF
FILL B FF
UPDT
```

Get compiled to the following _Binary format_:
```
04 00
0D 00
0F FF	
14 FF
15 FF
16 FF
00 00
```

### Basic Instructions
| Name   | Instruction | Options                      | Value         | Description                                               |
| ------ | ----------- | ---------------------------- | ------------- | --------------------------------------------------------- |
| Update | 0x00	UPDT   | None                         | None          | Updates the led strip with the [buffer](#buffer) contents |
| Clear  | 0x01	CLR    | None                         | None          | Clears the buffer, triggering all leds off                |
| Sleep  | 0x07 SLP    | [TimeOptions](#time-options) | Time to sleep | Delay execution for specified amount of time              |

#### Time Options
| Name         | Option   | Description       |
| ------------ | -------- | ----------------- |
| Milliseconds | 0x01 MS  | Sleep time in mS  |
| Seconds      | 0x02 SEC | Sleep time in S   |
| Minutes      | 0x03 MIN | Sleep time in min |

### Selection Instructions
Select which part of the [buffer](#buffer) will be modified

| Name         | Instruction | Options                          | Value                      | Description                                                                                                                    |
| ------------ | ----------- | -------------------------------- | -------------------------- | ------------------------------------------------------------------------------------------------------------------------------ |
| Select LED   | 0x02 SEL    | [SelectOptions](#select-options) | Absolute or Relative index | Select an specific LED by index (aboslute or relative. See [Absolute and Relative indexing](#absolute-and-relative-selection)) |
| Range Select | 0x03 RAN    | [RangeOptions](#range-options)   | Absolute or Relative index | Select a range of LEDs (aboslute or relative. See [Absolute and Relative indexing](#absolute-and-relative-selection))          |

#### Absolute and Relative indexing
Values in the [buffer](#buffer) can be indexed by their exact position called *Absolute Indexing* or, if the number of LEDs is unknown, their position can be specified from 0% to 100% which is called *Relative Indexing*

- *Absolute Indexing* is achieved by using the **#** operator. The beginning of a block is specified with **#** and the end of it with **##**.
- *Relative Indexing* is achieved by using the **%** operator. The beginning of a block is specified with **%** and the end of it with **%%**.

> ⚠️ **Warning**: _Aboslute Indexing_ causes undefined or implementation dependant behaviour on the interpreter when the index is greater than the [buffer](#buffer) size. If the number of LEDs is unknown and the register values cannot be obtained use only _Relative Indexing_

#### Select Options
| Name     | Option | Description                                     |
| -------- | ------ | ----------------------------------------------- |
| Absolute | 0x00 # | Exact index of the LED in strip                 |
| Relative | 0x01 % | Relative position from 0% (0x00) to 100% (0xFF) |

#### Range Options
| Name           | Option  | Description                                     |
| -------------- | ------- | ----------------------------------------------- |
| Absolute Start | 0x00 #  | Exact index of the range start (inclusive)      |
| Relative Start | 0x01 %  | Value from 0% (0x00) to 100% (0xFF) (inclusive) |
| Absolute End   | 0x02 ## | Exact index of the range end (inclusive)        |
| Relative End   | 0x03 %% | Value from 0% (0x00) to 100% (0xFF) (inclusive) |

### Color Instructions
Manipulate the RGB value inside the selected [buffer](#buffer) range or selection.

| Name       | Instruction | Options                        | Value           | Description                                                                            |
| ---------- | ----------- | ------------------------------ | --------------- | -------------------------------------------------------------------------------------- |
| Set color  | 0x04	SET    | [ColorOptions](#color-options) | Color HEX value | Set the specified color to the currently selected position with [SET](#select-options) |
| Fill range | 0x05	FILL   | [ColorOptions](#color-options) | Color HEX value | Set the specified color to the currently selected range with [RAN](#range-options)     |

#### Color options
| Name  | Option | Description     |
| ----- | ------ | --------------- |
| Red   | 0x00 R | RED hex value   |
| Green | 0x01 G | GREEN hex value |
| Blue  | 0x02 B | BLUE hex value  |

### Effects Instructions
| Name | Instruction | Options                          | Value            | Description                                             |
| ---- | ----------- | -------------------------------- | ---------------- | ------------------------------------------------------- |
| Blur | 0x06 EBLR   | [EffectOptions](#effect-options) | Effect intensity | Applies color blending/blurring to the strip or a range |

#### Effect options
Apply effects directly on a portion of the [buffer](#buffer)

| Name        | Option   | Description                            |
| ----------- | -------- | -------------------------------------- |
| Apply all   | 0x00 ALL | Apply the effect to the whole strip    |
| Apply range | 0x01 RAN | Apply the effect to the selected range |

### Load Instructions
Manipulation of the inner [registers](#registers)

| Name                         | Instruction | Options                      | Value                                                                                                                                     | Description                                                                    |
| ---------------------------- | ----------- | ---------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------ |
| Load a value into X register | 0x08 LDX    | [LoadOptions](#load-options) | Buffer [Absolute Index](#absolute-and-relative-indexing) to grab the value from or empty when grabbing [S constant](#size-number-of-leds) | Load an RGB value from the [buffer](#buffer) or the S constant into register X |
| Load a value into Y register | 0x09 LDY    | [LoadOptions](#load-options) | Buffer [Absolute Index](#absolute-and-relative-indexing) to grab the value from or empty when grabbing [S constant](#size-number-of-leds) | Load an RGB value from the [buffer](#buffer) or the S constant into register Y |

#### Load options

| Name       | Option | Description                                                         |
| ---------- | ------ | ------------------------------------------------------------------- |
| Load Size  | 0x00 S | Load the [buffer size](#size-number-of-leds) (No value is required) |
| Load RED   | 0x01 R | Load the 8-bit RED portion of the [buffer](#buffer)                 |
| Load GREEN | 0x02 G | Load the 8-bit GREEN portion of the [buffer](#buffer)               |
| Load BLUE  | 0x03 B | Load the 8-bit BLUE portion of the [buffer](#buffer)                |

### Other Instructions
| Name           | Instruction | Options | Value | Description                                                                                                                     |
| -------------- | ----------- | ------- | ----- | ------------------------------------------------------------------------------------------------------------------------------- |
| Null Operation | 0xFE	NOP    | None    | None  | (Does nothing) Set by [newton](#newton) when an empty line or a comment is found, can also be specified with the `NOP`` literal |
| EXCEPTION      | 0xFF        | None    | None  | (Aborts execution) Set by [newton](#newton) when an unrecognized instruction literal is found                                   |

## Standard Communication Protocols
### Bluetooth (BLE)
the _Prism Service_ only contains one characteristic which is the _Newton Characteristic_, this characteristic's value is 16-bit and is Read & Write enabled

When the characteristic is written (in _16-bit Big Endian_ format), the value is interpreted as an [instruction](#instruction-set). Otherwise, when the characteristic is read, the X (MSB) and Y (LSB) registers are returned concatenated in _Big Endian_ format

#### GATT Description
- Prism Service UUID: `a7a37338-0c20-4779-b75a-089c6d7a0ac8`
- Newton Characteristic
  - __UUID:__ `4e639365-9e62-4d81-8e3d-f0d2bde4ccc6`
  - __Type:__ READ and WRITE
  - __Unit:__ 0x2700 - unitless
  - __Format:__ 0x06 - uint16 (Big Endian)

## Newton
Newton is the name given to the _Prism instruction interpreter_ therefore a _Newton Interpreter_ is required in every [slave](#slave) device.

a [FastLED](http://fastled.io/) newton compatible (C++ header-only) interpreter library is bundled ready to use `/fastled/include/newton_fastled.hxx`. Usage examples  can be found in `/fastled/examples`

### libnewton
A C header-only library is bundled (`/libnewton/include`) and contains all the Newton interpreter definitions. Include the header with

Include the whole library with:
```c
#include <newton.h>
```

Or include only the required modules with
```c
#include <newton/registers.h>
#include <newton/instructions.h>
#include <newton/parser.h>
```
- `/libnewton/include/newton/parser.h` constains functions for _Instruction Literal Parsing_, _Binary Format Parsing_ and _Binary Format Writing_.

```c
PrismInstruction Newton_ParseInstructionU16(uint16_t byte);
PrismInstruction Newton_ParseInstructionLiteral(const char *instruction);
uint16_t Newton_WriteInstructionToU16(const PrismInstruction instruction);
```

- `/libnewton/include/newton/instructions.h`
Contains all the Newton definitions like the [Prism Instruction Set](#instruction-set) and the corresponding options.

```c
typedef struct {
  PrismInstructionSet instruction;
  PrismInstructionOptions options;
  uint8_t value;
} PrismInstruction;
```

```c
typedef enum {
  PRISM_INSTRUCTION_UPDATE = 0x00,
  /* ...  */
  PRISM_INSTRUCTION_LOADY = 0x09,
  PRISM_IGNORE_INSTRUCTION = 0xFE,
  PRISM_EXCEPTION = 0xFF,
} PrismInstructionSet;
```
```c
typedef union {
  struct NoOptions;
  enum {
    PRISM_OPTION_SELECT_ABSOLUTE = 0,
    PRISM_OPTION_SELECT_RELATIVE = 1
  } SelectOptions;
  enum /* ... */ RangeOptions;
  enum /* ... */ ColorOptions;
  enum /* ... */ EffectOptions;
  enum /* ... */ TimeOptions;
  enum /* ... */ LoadOptions;
} PrismInstructionOptions;
```

- `/libnewton/include/newton/registers.h` Contains the structure with the [registers](#registers)

```c
typedef struct {
  uint8_t X;
  uint8_t Y;
} NewtonRegisters;
```