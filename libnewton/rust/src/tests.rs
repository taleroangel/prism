use crate::core::{
    instruction::{Instruction, InstructionSet},
    options::{InstructionOptions, SelectOptions, RangeOptions, ColorOptions, LoadOptions, EffectOptions, TimeOptions},
};

fn assert_instruction(binary: u16, instruction: Instruction) {
    // Binary to instruction
    let expected_instruction = Instruction::parse_from_u16(binary).unwrap();
    assert_eq!(expected_instruction, instruction);

    // Instruction to binary
    let expected_binary = instruction.write_to_u16();
    assert_eq!(expected_binary, binary);
}

#[test]
pub fn instruction_parsing_from_u16() {
    // Update
    assert_instruction(
        0b0000000011111111,
        Instruction {
            instruction: InstructionSet::Update,
            options: InstructionOptions::None,
            value: 0xFF,
        },
    );

    // Clear
    assert_instruction(
        0b0000010011111111,
        Instruction {
            instruction: InstructionSet::Clear,
            options: InstructionOptions::None,
            value: 0xFF,
        },
    );

    // Select absolute
    assert_instruction(
        0b0000100010101010,
        Instruction {
            instruction: InstructionSet::Select,
            options: InstructionOptions::Select(SelectOptions::Absolute),
            value: 0xAA,
        },
    );

	// Select relative
	assert_instruction(
        0b0000100110101010,
        Instruction {
            instruction: InstructionSet::Select,
            options: InstructionOptions::Select(SelectOptions::Relative),
            value: 0xAA,
        },
    );
  
	// Range absolute start
	assert_instruction(
        0b0000110010101010,
        Instruction {
            instruction: InstructionSet::Range,
            options: InstructionOptions::Range(RangeOptions::AbsoluteStart),
            value: 0xAA,
        },
    );
  
	// Range absolute end
	assert_instruction(
        0b0000111011111111,
        Instruction {
            instruction: InstructionSet::Range,
            options: InstructionOptions::Range(RangeOptions::AbsoluteEnd),
            value: 0xFF,
        },
    );
  
	// Range relative start
	assert_instruction(
        0b0000110110101010,
        Instruction {
            instruction: InstructionSet::Range,
            options: InstructionOptions::Range(RangeOptions::RelativeStart),
            value: 0xAA,
        },
    );
  
	// Range relative end
	assert_instruction(
        0b0000111111111111,
        Instruction {
            instruction: InstructionSet::Range,
            options: InstructionOptions::Range(RangeOptions::RelativeEnd),
            value: 0xFF,
        },
    );
  
	// Fill Color R/H
	assert_instruction(
        0b0001010011110000,
        Instruction {
            instruction: InstructionSet::Fill,
            options: InstructionOptions::Color(ColorOptions::Red),
            value: 0xF0,
        },
    );
  
	// Fill Color G/S
	assert_instruction(
        0b0001010111110000,
        Instruction {
            instruction: InstructionSet::Fill,
            options: InstructionOptions::Color(ColorOptions::Green),
            value: 0xF0,
        },
    );
  
	// Fill Color B/V
	assert_instruction(
        0b0001011011110000,
        Instruction {
            instruction: InstructionSet::Fill,
            options: InstructionOptions::Color(ColorOptions::Blue),
            value: 0xF0,
        },
    );
  
	// Set Color R/H
	assert_instruction(
        0b0001000011110000,
        Instruction {
            instruction: InstructionSet::Set,
            options: InstructionOptions::Color(ColorOptions::Red),
            value: 0xF0,
        },
    );
  
	// Set Color G/S
	assert_instruction(
        0b0001000111110000,
        Instruction {
            instruction: InstructionSet::Set,
            options: InstructionOptions::Color(ColorOptions::Green),
            value: 0xF0,
        },
    );
  
	// Set Color B/V
	assert_instruction(
        0b0001001011110000,
        Instruction {
            instruction: InstructionSet::Set,
            options: InstructionOptions::Color(ColorOptions::Blue),
            value: 0xF0,
        },
    );
  
	// Set register X with S
	assert_instruction(
        0b0010000000000001,
        Instruction {
            instruction: InstructionSet::LoadX,
            options: InstructionOptions::Load(LoadOptions::Variable),
            value: 0x01,
        },
    );
  
	// Set register X with V
	assert_instruction(
        0b0010000000000000,
        Instruction {
            instruction: InstructionSet::LoadX,
            options: InstructionOptions::Load(LoadOptions::Variable),
            value: 0x00,
        },
    );
  
	// Set register X with R FF
	assert_instruction(
        0b0010000111111111,
        Instruction {
            instruction: InstructionSet::LoadX,
            options: InstructionOptions::Load(LoadOptions::Red),
            value: 0xFF,
        },
    );
  
	// Set register X with G FF
	assert_instruction(
        0b0010001011111111,
        Instruction {
            instruction: InstructionSet::LoadX,
            options: InstructionOptions::Load(LoadOptions::Green),
            value: 0xFF,
        },
    );
  
	// Set register X with R FF
	assert_instruction(
        0b0010001111111111,
        Instruction {
            instruction: InstructionSet::LoadX,
            options: InstructionOptions::Load(LoadOptions::Blue),
            value: 0xFF,
        },
    );
  
	// Set register Y with V
	assert_instruction(
        0b0010010000000000,
        Instruction {
            instruction: InstructionSet::LoadY,
            options: InstructionOptions::Load(LoadOptions::Variable),
            value: 0x00,
        },
    );
  
	// Set register Y with S
	assert_instruction(
        0b0010010000000001,
        Instruction {
            instruction: InstructionSet::LoadY,
            options: InstructionOptions::Load(LoadOptions::Variable),
            value: 0x01,
        },
    );
  
	// Set register Y with R FF
	assert_instruction(
        0b0010010111111111,
        Instruction {
            instruction: InstructionSet::LoadY,
            options: InstructionOptions::Load(LoadOptions::Red),
            value: 0xFF,
        },
    );
  
	// Set register Y with G FF
	assert_instruction(
        0b0010011011111111,
        Instruction {
            instruction: InstructionSet::LoadY,
            options: InstructionOptions::Load(LoadOptions::Green),
            value: 0xFF,
        },
    );
  
	// Set register Y with R FF
	assert_instruction(
        0b0010011111111111,
        Instruction {
            instruction: InstructionSet::LoadY,
            options: InstructionOptions::Load(LoadOptions::Blue),
            value: 0xFF,
        },
    );
  
	// Blur All
	assert_instruction(
        0b0001100011110000,
        Instruction {
            instruction: InstructionSet::Blur,
            options: InstructionOptions::Effect(EffectOptions::ApplyAll),
            value: 0xF0,
        },
    );
  
	// Blur Range
	assert_instruction(
        0b0001100111110000,
        Instruction {
            instruction: InstructionSet::Blur,
            options: InstructionOptions::Effect(EffectOptions::ApplyRange),
            value: 0xF0,
        },
    );
  
	// Sleep (ms)
	assert_instruction(
        0b0001110111110000,
        Instruction {
            instruction: InstructionSet::Sleep,
            options: InstructionOptions::Time(TimeOptions::Ms),
            value: 0xF0,
        },
    );
  
	// Sleep (sec)
	assert_instruction(
        0b0001111011110000,
        Instruction {
            instruction: InstructionSet::Sleep,
            options: InstructionOptions::Time(TimeOptions::Sec),
            value: 0xF0,
        },
    );
  
	// Sleep (min)
	assert_instruction(
        0b0001111111110000,
        Instruction {
            instruction: InstructionSet::Sleep,
            options: InstructionOptions::Time(TimeOptions::Min),
            value: 0xF0,
        },
    );
}
