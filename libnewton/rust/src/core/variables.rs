use num_enum::TryFromPrimitive;

#[derive(TryFromPrimitive)]
#[repr(u8)]
pub enum VariablesCodes {
    Version = 0x00,
    BufferSize = 0x01,
}

pub struct Variables {
    pub buffer_size: u8,
    _newton_version: u8,
}

impl Variables {
    pub fn new(buffer_size: u8) -> Self {
        Self {
            buffer_size,
            _newton_version: crate::LIBNEWTON_VERSION,
        }
    }

    pub fn newton_version(&self) -> u8 {
        self._newton_version
    }
}
