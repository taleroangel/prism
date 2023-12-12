use super::variables::Variables;

#[derive(Debug, Clone, Copy)]
pub struct Registers {
    pub x: u8,
    pub y: u8,
}

impl Registers {
    pub fn new(variables: Variables) -> Self {
        Self {
            x: variables.newton_version(),
            y: variables.buffer_size,
        }
    }
}
