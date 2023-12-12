use super::instruction::RawInstruction;

#[derive(Debug)]
pub enum NewtonError {
    InvalidInstructionError { raw_instruction: RawInstruction },
    InvalidInstructionLiteral { instruction_literal: String },
}
