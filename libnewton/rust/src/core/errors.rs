use super::{instruction::{RawInstruction, InstructionSet}, options::InstructionOptions};

#[derive(Debug)]
pub enum NewtonError {
    InvalidInstructionError { raw_instruction: RawInstruction },
    InvalidInstructionLiteral { instruction_literal: String },
    InvalidOptionForInstruction { instruction: InstructionSet, option: InstructionOptions },
    InterpreterException,
	InvalidVariableCode(u8),
}
