# Prism

## Registers

## Instruction Set

### Basic Instructions
| Name   | Instruction | Options                      | Value         | Description                                    |
| ------ | ----------- | ---------------------------- | ------------- | ---------------------------------------------- |
| Update | 0x00	UPDT   | None                         | None          | Updates the led strip with the buffer contents |
| Clear  | 0x01	CLR    | None                         | None          | Clears the buffer, triggering all leds off     |
| Sleep  | 0x07 SLP    | [TimeOptions](#time-options) | Time to sleep | Delay execution for specified amount of time   |

#### Time Options
| Name         | Option   | Description       |
| ------------ | -------- | ----------------- |
| Microseconds | 0x00 US  | Sleep time in µS  |
| Milliseconds | 0x01 MS  | Sleep time in mS  |
| Seconds      | 0x02 SEC | Sleep time in S   |
| Minutes      | 0x03 MIN | Sleep time in min |

### Selection Instructions
| Name         | Instruction | Options                          | Value                      | Description                                                                                                                     |
| ------------ | ----------- | -------------------------------- | -------------------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| Select LED   | 0x02 SEL    | [SelectOptions](#select-options) | Absolute or relative index | Select an specific LED by index (aboslute or relative. See [Absolute and Relative selection](#absolute-and-relative-selection)) |
| Range Select | 0x03 RAN    | [RangeOptions](#range-options)   | Absolute or relative index | Select a range of LEDs (aboslute or relative. See [Absolute and Relative selection](#absolute-and-relative-selection))          |

#### Absolute and Relative selection
TODO

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
| Name       | Instruction | Options                        | Value           | Description                                                     |
| ---------- | ----------- | ------------------------------ | --------------- | --------------------------------------------------------------- |
| Set color  | 0x04	SET    | [ColorOptions](#color-options) | Color HEX value | Set the specified color to the currently selected LED           |
| Fill range | 0x05	FILL   | [ColorOptions](#color-options) | Color HEX value | Set the specified color to the currently selected range of LEDs |

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
| Name        | Option   | Description                            |
| ----------- | -------- | -------------------------------------- |
| Apply all   | 0x00 ALL | Apply the effect to the whole strip    |
| Apply range | 0x01 RAN | Apply the effect to the selected range |

### Register Instructions
| Name | Options | Description |
| Load into public | | |

### Other Instructions
| Name           | Instruction | Options | Value | Description                                                                                                                     |
| -------------- | ----------- | ------- | ----- | ------------------------------------------------------------------------------------------------------------------------------- |
| Null Operation | 0xFE	NOP    | None    | None  | (Does nothing) Set by [newton](#newton) when an empty line or a comment is found, can also be specified with the `NOP`` literal |
| EXCEPTION      | 0xFF        | None    | None  | (Aborts execution) Set by [newton](#newton) when an unrecognized instruction literal is found                                   |

## Newton

## Bluetooth BLE
Service UUID:

	a7a37338-0c20-4779-b75a-089c6d7a0ac8