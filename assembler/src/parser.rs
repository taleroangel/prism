use std::collections::HashMap;
use logos::Lexer;
use thiserror::Error;
use anyhow::Result;
use libnewton::addressing::{AddressingMode};
use libnewton::arrays::{Array, Array2, Array3};
use libnewton::codes::{DelayCode, EffectCode};
use libnewton::instruction::InstructionSet;
use libnewton::registers::Register;
use regex::Regex;
use crate::lexer::PrismToken;

#[derive(Debug, Error)]
pub enum PrismParsingError {
    #[error("register `{0}` not recognized")]
    InvalidRegister(String),
    #[error("failed lex, token not identified")]
    FailedLex,
    #[error("unexpected token `{0:?}` found")]
    UnexpectedToken(PrismToken),
    #[error("unexpected EOF during parsing")]
    UnexpectedEOF,
    #[error("invalid array size, arrays can only be Array2(range) and Array3(color) types")]
    InvalidArraySize,
    #[error("undefined label `{0}` used")]
    UndefinedLabel(String),
    #[error("array can only have one addressing type")]
    MixedArrayAddressing,
    #[error("invalid type `{0}`, expected `{1}`")]
    InvalidType(String, String),
    #[error("unrecongized instruction `{0}`")]
    UnrecognizedInstruction(String),
    #[error("unrecognized delay type `{0}`")]
    InvalidDelayType(String),
}

/// Parse the register name
fn parse_register_name(register: &str) -> Option<Register> {
    match register {
        "sc" => Some(Register::SC),
        "sf" => Some(Register::SF),
        "pc" => Some(Register::PC),
        "pp" => Some(Register::PP),
        "rv" => Some(Register::RV),
        "r0" => Some(Register::R0),
        "r1" => Some(Register::R1),
        "po" => Some(Register::PO),
        _ => None,
    }
}

/// Parse the delay type
fn parse_delay_code(delay: &str) -> Option<DelayCode> {
    match delay {
        "ms" => Some(DelayCode::MS),
        "sec" => Some(DelayCode::SEC),
        "min" => Some(DelayCode::MIN),
        "hrs" => Some(DelayCode::HRS),
        _ => None,
    }
}

/// Parse memory addressing mode
fn parse_addressing(token: PrismToken) -> Result<AddressingMode> {
    match token {
        PrismToken::Number(n) => Ok(AddressingMode::Immediate(n)),
        PrismToken::Register(r) => Ok(AddressingMode::Indirect(
            parse_register_name(r.as_str())
                .ok_or(PrismParsingError::InvalidRegister(r))?)),
        PrismToken::GeneralPurposeRegister(r) =>
            Ok(AddressingMode::Indirect(Register::GeneralPurpose(r))),
        _ => Err(PrismParsingError::UnexpectedToken(token).into()),
    }
}

// Parse a well-known register or a general purpose register
fn parse_register(token: PrismToken) -> Result<Register> {
    match token {
        PrismToken::Register(r) => Ok(
            parse_register_name(r.as_str())
                .ok_or(PrismParsingError::InvalidRegister(r))?),
        PrismToken::GeneralPurposeRegister(r) =>
            Ok(Register::GeneralPurpose(r)),
        _ => Err(PrismParsingError::UnexpectedToken(token).into()),
    }
}

// Parse a numeric value (usually a code
fn parse_numeric(token: PrismToken) -> Result<u8> {
    match token {
        PrismToken::Number(n) => Ok(n),
        _ => Err(PrismParsingError::UnexpectedToken(token).into()),
    }
}

/// Unwrap tokens
fn next_token(lexer: &mut Lexer<PrismToken>) -> Result<PrismToken> {
    lexer.next().ok_or(PrismParsingError::UnexpectedEOF)?
        .map_err(|_| PrismParsingError::FailedLex.into())
}

/// Parse an array
fn parse_next_as_array(lexer: &mut Lexer<PrismToken>) -> Result<Array> {

    // Store the array contents
    let mut contents: Vec<AddressingMode> = vec![];

    // Marks the beggining of a new array
    let mut matcharray = false;

    loop {
        let token = next_token(lexer)?;
        match token {
            PrismToken::ArrayBegin => matcharray = true,
            PrismToken::ArrayEnd => break,

            PrismToken::Register(_) |
            PrismToken::Number(_) |
            PrismToken::GeneralPurposeRegister(_) =>
                if matcharray {
                    contents.push(parse_addressing(token)?)
                } else {
                    return Err(PrismParsingError::UnexpectedToken(token).into());
                }

            // This token is not expected for an array
            _ => return Err(PrismParsingError::UnexpectedToken(token).into())
        }
    };

    // Check for array type
    match contents.first().ok_or(PrismParsingError::InvalidArraySize)? {
        AddressingMode::Immediate(_) => {
            // Check that every value is also immediate
            if contents.iter().any(move |x| match x {
                AddressingMode::Immediate(_) => false,
                AddressingMode::Indirect(_) => true,
            }) {
                return Err(PrismParsingError::MixedArrayAddressing.into());
            }
        }
        AddressingMode::Indirect(_) => {
            // Check that every value is also indirect
            if contents.iter().any(move |x| match x {
                AddressingMode::Immediate(_) => true,
                AddressingMode::Indirect(_) => false,
            }) {
                return Err(PrismParsingError::MixedArrayAddressing.into());
            }
        }
    };

    // Match array type
    match contents.len() {
        2 => Ok(Array::Range(Array2::try_from(&contents).map_err(|_| PrismParsingError::InvalidArraySize)?)),
        3 => Ok(Array::Color(Array3::try_from(&contents).map_err(|_| PrismParsingError::InvalidArraySize)?)),
        _ => Err(PrismParsingError::InvalidArraySize.into())
    }
}

/// Get a parsed array into Arr2 format
fn get_array2(array: Array) -> Result<Array2<AddressingMode>> {
    match array {
        Array::Range(a) => Ok(a),
        Array::Color(_) => Err(PrismParsingError::InvalidType(
            "Array3".to_string(),
            "Array2".to_string(),
        ).into())
    }
}

/// Get a parsed array into Arr3 format
fn get_array3(array: Array) -> Result<Array3<AddressingMode>> {
    match array {
        Array::Color(a) => Ok(a),
        Array::Range(_) => Err(PrismParsingError::InvalidType(
            "Array2".to_string(),
            "Array3".to_string(),
        ).into())
    }
}

/// Parse a label and put the exact memory location to jump at
fn parse_next_as_label(lexer: &mut Lexer<PrismToken>, labels: &HashMap<String, usize>) -> Result<u8> {
    // Get the current token
    let token = next_token(lexer)?;

    // Match token type
    match token {
        // Parse label
        PrismToken::Text(label) => if let Some(&pc) =
            labels.get(&label)
        { Ok(pc as u8) } else {
            Err(PrismParsingError::UndefinedLabel(label).into())
        }

        // Not a label
        _ => Err(PrismParsingError::UnexpectedToken(token).into())
    }
}

/// Identify the labels present in the source code
pub fn identify_labels(source: &str) -> Result<HashMap<String, usize>> {
    let regex = Regex::new(r"[a-z0-9]+:")?;
    // Split source into instructions
    Ok(source.split("\n").enumerate()
        .filter(move |&(_, x)| regex.is_match(x))
        // Remove trailing dots and put label as the key
        .map(move |(it, x)| (x[..x.len() - 1].to_string(), it)).collect())
}

/// Parse a prism file and return a vector of instructions
pub fn parsepf(lexer: &mut Lexer<PrismToken>, labels: HashMap<String, usize>) -> Result<Vec<InstructionSet>> {
    // Get the list of instructions
    let mut instructions: Vec<InstructionSet> = vec![];

    while let Some(token) = lexer.next() {
        // On pattern matched
        if let Ok(token) = token {
            // First level of the hierarchy
            match token {
                // Ignore comments or empty newlines
                PrismToken::LineComment | PrismToken::Newline => {}
                // Instruction
                PrismToken::Text(instruction) =>
                    instructions.push(match instruction.as_str() {
                        "nop" => InstructionSet::NOP,
                        "begin" => InstructionSet::BEGIN,
                        "run" => InstructionSet::RUN,
                        "transmit" => InstructionSet::TRANSMIT,
                        "halt" => InstructionSet::HALT(parse_addressing(next_token(lexer)?)?),
                        "aidx" => InstructionSet::AIDX,
                        "ridx" => InstructionSet::RIDX,
                        "hold" => InstructionSet::HOLD,
                        "nhold" => InstructionSet::NHOLD,
                        "update" => InstructionSet::UPDATE,
                        "jmp" => InstructionSet::JMP(parse_next_as_label(lexer, &labels)?),
                        "ret" => InstructionSet::RET(parse_addressing(next_token(lexer)?)?),
                        "beq" => InstructionSet::BEQ(
                            parse_addressing(next_token(lexer)?)?,
                            parse_addressing(next_token(lexer)?)?,
                            parse_next_as_label(lexer, &labels)?,
                        ),
                        "bne" => InstructionSet::BNE(
                            parse_addressing(next_token(lexer)?)?,
                            parse_addressing(next_token(lexer)?)?,
                            parse_next_as_label(lexer, &labels)?,
                        ),
                        "load" => InstructionSet::LOAD(
                            parse_register(next_token(lexer)?)?,
                            parse_addressing(next_token(lexer)?)?,
                        ),
                        "add" => InstructionSet::ADD(
                            parse_register(next_token(lexer)?)?,
                            parse_addressing(next_token(lexer)?)?,
                        ),
                        "sub" => InstructionSet::SUB(
                            parse_register(next_token(lexer)?)?,
                            parse_addressing(next_token(lexer)?)?,
                        ),
                        "fill" => InstructionSet::FILL(
                            get_array2(parse_next_as_array(lexer)?)?,
                            get_array3(parse_next_as_array(lexer)?)?,
                        ),
                        "hfill" => InstructionSet::HFILL(
                            get_array2(parse_next_as_array(lexer)?)?,
                            parse_addressing(next_token(lexer)?)?,
                        ),
                        "sfill" => InstructionSet::SFILL(
                            get_array2(parse_next_as_array(lexer)?)?,
                            parse_addressing(next_token(lexer)?)?,
                        ),
                        "lfill" => InstructionSet::LFILL(
                            get_array2(parse_next_as_array(lexer)?)?,
                            parse_addressing(next_token(lexer)?)?,
                        ),
                        "paint" => InstructionSet::PAINT(
                            parse_addressing(next_token(lexer)?)?,
                            get_array3(parse_next_as_array(lexer)?)?,
                        ),
                        "hpaint" => InstructionSet::HPAINT(
                            parse_addressing(next_token(lexer)?)?,
                            parse_addressing(next_token(lexer)?)?,
                        ),
                        "spaint" => InstructionSet::SPAINT(
                            parse_addressing(next_token(lexer)?)?,
                            parse_addressing(next_token(lexer)?)?,
                        ),
                        "lpaint" => InstructionSet::LPAINT(
                            parse_addressing(next_token(lexer)?)?,
                            parse_addressing(next_token(lexer)?)?,
                        ),
                        "effect" => InstructionSet::EFFECT(
                            EffectCode(parse_numeric(next_token(lexer)?)?),
                            get_array2(parse_next_as_array(lexer)?)?,
                            parse_addressing(next_token(lexer)?)?,
                        ),
                        "delay" => InstructionSet::DELAY(
                            match next_token(lexer)? {
                                PrismToken::Text(delay) => parse_delay_code(
                                    delay.as_str()).ok_or(PrismParsingError::InvalidDelayType(delay)),
                                _ => Err(PrismParsingError::FailedLex)
                            }?,
                            parse_addressing(next_token(lexer)?)?,
                        ),
                        "pause" => InstructionSet::PAUSE,
                        "get" => InstructionSet::GET(parse_register(next_token(lexer)?)?),
                        "reset" => InstructionSet::RESET,
                        _ => return Err(PrismParsingError::UnrecognizedInstruction(instruction).into())
                    }),

                PrismToken::LabelDeclaration(label) => {
                    if !labels.contains_key(&label) {
                        return Err(PrismParsingError::UndefinedLabel(label).into());
                    } else {
                        // Add NOP so the label index is kept
                        instructions.push(InstructionSet::NOP)
                    }
                }
                _ => return Err(PrismParsingError::UnexpectedToken(token).into())
            }
        } else {
            return Err(PrismParsingError::FailedLex.into());
        }
    }

    // Return the instruction vector
    Ok(instructions)
}