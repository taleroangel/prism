use num_enum::TryFromPrimitive;

use super::options::InstructionOptions;

/// Instructions codes
#[derive(Clone, Copy, TryFromPrimitive, PartialEq, Eq, Debug)]
#[repr(u8)]
pub enum InstructionSet {
    /* Control instructions */
    Update = 0x00,
    Clear = 0x01,
    Sleep = 0x07,

    /* Selection instructions */
    Select = 0x02,
    Range = 0x03,

    /* Buffer alteration instructions */
    Set = 0x04,
    Fill = 0x05,

    /* Effects */
    Blur = 0x06,

    /* Register instructions */
    LoadX = 0x08,
    LoadY = 0x09,

    /* Misc */
    Ignore = 0xFE,
    Exception = 0xFF,
}

#[derive(Clone, Copy, Debug)]
pub struct RawInstruction {
    pub instruction: u8,
    pub options: u8,
    pub value: u8,
}

#[derive(Clone, PartialEq, Eq, Debug)]
pub struct Instruction {
    pub instruction: InstructionSet,
    pub options: InstructionOptions,
    pub value: u8,
}
