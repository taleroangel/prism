use num_enum::TryFromPrimitive;

#[derive(TryFromPrimitive)]
#[repr(u8)]
pub enum VariablesCodes {
    Version = 0x00,
    BufferSize = 0x01,
}

#[derive(Debug, Clone, Copy)]
pub struct Variables {
    pub buffer_size: u8,
    newton_version: u8,
}

impl Variables {
    pub fn new(buffer_size: u8) -> Self {
        Self {
            buffer_size,
            newton_version: crate::LIBNEWTON_VERSION,
        }
    }

    pub fn newton_version(&self) -> u8 {
        self.newton_version
    }
}
