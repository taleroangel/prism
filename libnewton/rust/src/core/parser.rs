use super::{
    errors::NewtonError,
    instruction::{Instruction, InstructionSet, RawInstruction},
    options::{
        ColorOptions, EffectOptions, InstructionOptions, LoadOptions, RangeOptions, SelectOptions,
        TimeOptions,
    },
};

impl Instruction {
    pub fn new_ignore() -> Self {
        Instruction {
            instruction: InstructionSet::Ignore,
            options: InstructionOptions::None,
            value: 0x00,
        }
    }

    pub fn parse_from_u16(bytes: u16) -> Result<Self, NewtonError> {
        // Get instruction bytes
        let raw_instruction = RawInstruction {
            instruction: ((bytes & 0xFC00) >> 10) as u8,
            options: ((bytes & 0x0300) >> 8) as u8,
            value: (bytes & 0x00FF) as u8,
        };

        let parsed_instruction = InstructionSet::try_from(raw_instruction.instruction)
            .map_err(|_| NewtonError::InvalidInstructionError { raw_instruction })?;

        let parsed_option = match parsed_instruction {
            InstructionSet::Update => InstructionOptions::None,
            InstructionSet::Clear => InstructionOptions::None,
            InstructionSet::Sleep => InstructionOptions::Time(
                TimeOptions::try_from(raw_instruction.options)
                    .map_err(|_| NewtonError::InvalidInstructionError { raw_instruction })?,
            ),
            InstructionSet::Select => InstructionOptions::Select(
                SelectOptions::try_from(raw_instruction.options)
                    .map_err(|_| NewtonError::InvalidInstructionError { raw_instruction })?,
            ),
            InstructionSet::Range => InstructionOptions::Range(
                RangeOptions::try_from(raw_instruction.options)
                    .map_err(|_| NewtonError::InvalidInstructionError { raw_instruction })?,
            ),
            InstructionSet::Set => InstructionOptions::Color(
                ColorOptions::try_from(raw_instruction.options)
                    .map_err(|_| NewtonError::InvalidInstructionError { raw_instruction })?,
            ),
            InstructionSet::Fill => InstructionOptions::Color(
                ColorOptions::try_from(raw_instruction.options)
                    .map_err(|_| NewtonError::InvalidInstructionError { raw_instruction })?,
            ),
            InstructionSet::Blur => InstructionOptions::Effect(
                EffectOptions::try_from(raw_instruction.options)
                    .map_err(|_| NewtonError::InvalidInstructionError { raw_instruction })?,
            ),
            InstructionSet::LoadX => InstructionOptions::Load(
                LoadOptions::try_from(raw_instruction.options)
                    .map_err(|_| NewtonError::InvalidInstructionError { raw_instruction })?,
            ),
            InstructionSet::LoadY => InstructionOptions::Load(
                LoadOptions::try_from(raw_instruction.options)
                    .map_err(|_| NewtonError::InvalidInstructionError { raw_instruction })?,
            ),
            InstructionSet::Ignore => InstructionOptions::None,
            InstructionSet::Exception => InstructionOptions::None,
        };

        // Get the parsed instruction
        let instruction = Instruction {
            instruction: parsed_instruction,
            options: parsed_option,
            value: raw_instruction.value,
        };

        return Ok(instruction);
    }

    pub fn write_to_u16(&self) -> u16 {
        let raw_instruction = RawInstruction {
            instruction: self.instruction as u8,
            options: match self.options {
                InstructionOptions::None => 0,
                InstructionOptions::Color(value) => value as u8,
                InstructionOptions::Effect(value) => value as u8,
                InstructionOptions::Load(value) => value as u8,
                InstructionOptions::Range(value) => value as u8,
                InstructionOptions::Select(value) => value as u8,
                InstructionOptions::Time(value) => value as u8,
            },
            value: self.value,
        };

        let mut instruction_bytes: u16 = 0x0000;

        instruction_bytes |= (raw_instruction.options as u16) << 8;
        instruction_bytes |= ((raw_instruction.instruction as u16) << 10) & 0xFF00;
        instruction_bytes |= (raw_instruction.value as u16) & 0x00FF;

        return instruction_bytes;
    }
}
