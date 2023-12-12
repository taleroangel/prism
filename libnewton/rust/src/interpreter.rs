use super::core::{
    errors::NewtonError,
    instruction::{Instruction, InstructionSet},
    options::{
        ColorOptions, InstructionOptions, LoadOptions, RangeOptions, SelectOptions, TimeOptions,
    },
    registers::Registers,
    variables::{Variables, VariablesCodes},
};

/// Represents a Buffer RGB value
#[derive(Debug, Clone, Copy)]
pub struct BuffItem {
    pub red: u8,
    pub green: u8,
    pub blue: u8,
}

impl BuffItem {
    pub fn new() -> Self {
        BuffItem {
            red: 0,
            green: 0,
            blue: 0,
        }
    }
}

/// Action to take by the interpreter
#[derive(Debug, Clone, Copy)]
pub enum InterpreterAction {
    NoAction,
    Update,
    Sleep(u64),
}

#[derive(Clone)]
pub struct Interpreter {
    pub public_buffer: Vec<BuffItem>,
    buffer: Vec<BuffItem>,
    buffer_size: u8,
    variables: Variables,
    registers: Registers,
    current_selection: u8,
    current_range: (u8, u8),
}

impl Interpreter {
    pub fn new(buffer_size: u8) -> Self {
        let variables = Variables::new(buffer_size);
        Self {
            public_buffer: vec![BuffItem::new(); buffer_size as usize],
            buffer: vec![BuffItem::new(); buffer_size as usize],
            buffer_size,
            variables: variables,
            registers: Registers::new(variables),
            current_selection: 0,
            current_range: (0, 0),
        }
    }

    // Transforms a relative index into an aboslute one
    fn map_relative_index(&self, relative_index: u8) -> u8 {
        (((relative_index as u16) * ((self.buffer_size - 1) as u16)) / (u8::MAX as u16)) as u8
    }

    /// Interpret an [Instruction], returns either a [NewtonError] or an [NewtonInterpreterAction]
    pub fn interpret(
        &mut self,
        instruction: &Instruction,
    ) -> Result<InterpreterAction, NewtonError> {
        match instruction.instruction {
            InstructionSet::Clear => {
                self.buffer = vec![BuffItem::new(); self.buffer_size as usize];
                Ok(InterpreterAction::NoAction)
            }

            InstructionSet::Update => {
                self.public_buffer = self.buffer.clone();
                Ok(InterpreterAction::Update)
            }

            InstructionSet::Sleep => match instruction.options {
                InstructionOptions::Time(time_type) => match time_type {
                    TimeOptions::Ms => Ok(InterpreterAction::Sleep(instruction.value as u64)),
                    TimeOptions::Sec => Ok(InterpreterAction::Sleep(
                        (instruction.value as u64) * 1_000,
                    )),
                    TimeOptions::Min => Ok(InterpreterAction::Sleep(
                        (instruction.value as u64) * 60_000,
                    )),
                },
                _ => Err(NewtonError::InvalidOptionForInstruction {
                    instruction: instruction.instruction,
                    option: instruction.options,
                }),
            },

            InstructionSet::Select => match instruction.options {
                InstructionOptions::Select(select_option) => match select_option {
                    SelectOptions::Absolute => {
                        self.current_selection = instruction.value;
                        Ok(InterpreterAction::NoAction)
                    }
                    SelectOptions::Relative => {
                        self.current_selection = self.map_relative_index(instruction.value);
                        Ok(InterpreterAction::NoAction)
                    }
                },
                _ => Err(NewtonError::InvalidOptionForInstruction {
                    instruction: instruction.instruction,
                    option: instruction.options,
                }),
            },

            InstructionSet::Range => match instruction.options {
                InstructionOptions::Range(range_value) => match range_value {
                    RangeOptions::AbsoluteStart => {
                        self.current_range.0 = instruction.value;
                        Ok(InterpreterAction::NoAction)
                    }
                    RangeOptions::AbsoluteEnd => {
                        self.current_range.1 = instruction.value;
                        Ok(InterpreterAction::NoAction)
                    }
                    RangeOptions::RelativeStart => {
                        self.current_range.0 = self.map_relative_index(instruction.value);
                        Ok(InterpreterAction::NoAction)
                    }
                    RangeOptions::RelativeEnd => {
                        self.current_range.1 = self.map_relative_index(instruction.value);
                        Ok(InterpreterAction::NoAction)
                    }
                },
                _ => Err(NewtonError::InvalidOptionForInstruction {
                    instruction: instruction.instruction,
                    option: instruction.options,
                }),
            },

            InstructionSet::Set => match instruction.options {
                InstructionOptions::Color(color_value) => match color_value {
                    ColorOptions::Blue => {
                        self.buffer[self.current_selection as usize].red = instruction.value;
                        Ok(InterpreterAction::NoAction)
                    }
                    ColorOptions::Green => {
                        self.buffer[self.current_selection as usize].green = instruction.value;
                        Ok(InterpreterAction::NoAction)
                    }
                    ColorOptions::Red => {
                        self.buffer[self.current_selection as usize].blue = instruction.value;

                        Ok(InterpreterAction::NoAction)
                    }
                },
                _ => Err(NewtonError::InvalidOptionForInstruction {
                    instruction: instruction.instruction,
                    option: instruction.options,
                }),
            },

            InstructionSet::Fill => match instruction.options {
                InstructionOptions::Color(color_value) => match color_value {
                    ColorOptions::Blue => {
                        for vit in self.current_range.0..=self.current_range.1 {
                            self.buffer[vit as usize].red = instruction.value;
                        }
                        Ok(InterpreterAction::NoAction)
                    }
                    ColorOptions::Green => {
                        for vit in self.current_range.0..self.current_range.1 {
                            self.buffer[vit as usize].green = instruction.value;
                        }
                        Ok(InterpreterAction::NoAction)
                    }
                    ColorOptions::Red => {
                        for vit in self.current_range.0..self.current_range.1 {
                            self.buffer[vit as usize].blue = instruction.value;
                        }

                        Ok(InterpreterAction::NoAction)
                    }
                },
                _ => Err(NewtonError::InvalidOptionForInstruction {
                    instruction: instruction.instruction,
                    option: instruction.options,
                }),
            },

            InstructionSet::LoadX => match instruction.options {
                InstructionOptions::Load(load_value) => match load_value {
                    LoadOptions::Red => {
                        self.registers.x = self.buffer[self.current_selection as usize].red;
                        Ok(InterpreterAction::NoAction)
                    }
                    LoadOptions::Green => {
                        self.registers.x = self.buffer[self.current_selection as usize].green;
                        Ok(InterpreterAction::NoAction)
                    }
                    LoadOptions::Blue => {
                        self.registers.x = self.buffer[self.current_selection as usize].blue;
                        Ok(InterpreterAction::NoAction)
                    }
                    LoadOptions::Variable => {
                        let variable_value = VariablesCodes::try_from(instruction.value)
                            .map_err(|_| NewtonError::InvalidVariableCode(instruction.value))?;

                        self.registers.x = match variable_value {
                            VariablesCodes::BufferSize => self.variables.buffer_size,
                            VariablesCodes::Version => self.variables.newton_version(),
                        };

                        Ok(InterpreterAction::NoAction)
                    }
                },
                _ => Err(NewtonError::InvalidOptionForInstruction {
                    instruction: instruction.instruction,
                    option: instruction.options,
                }),
            },

            InstructionSet::LoadY => match instruction.options {
                InstructionOptions::Load(load_value) => match load_value {
                    LoadOptions::Red => {
                        self.registers.y = self.buffer[self.current_selection as usize].red;
                        Ok(InterpreterAction::NoAction)
                    }
                    LoadOptions::Green => {
                        self.registers.y = self.buffer[self.current_selection as usize].green;
                        Ok(InterpreterAction::NoAction)
                    }
                    LoadOptions::Blue => {
                        self.registers.y = self.buffer[self.current_selection as usize].blue;
                        Ok(InterpreterAction::NoAction)
                    }
                    LoadOptions::Variable => {
                        let variable_value = VariablesCodes::try_from(instruction.value)
                            .map_err(|_| NewtonError::InvalidVariableCode(instruction.value))?;

                        self.registers.y = match variable_value {
                            VariablesCodes::BufferSize => self.variables.buffer_size,
                            VariablesCodes::Version => self.variables.newton_version(),
                        };

                        Ok(InterpreterAction::NoAction)
                    }
                },
                _ => Err(NewtonError::InvalidOptionForInstruction {
                    instruction: instruction.instruction,
                    option: instruction.options,
                }),
            },

            InstructionSet::Blur => panic!("Effects are not yet implemented"),

            InstructionSet::Ignore => Ok(InterpreterAction::NoAction),
            InstructionSet::Exception => Err(NewtonError::InterpreterException),
        }
    }
}
