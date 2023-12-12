use num_enum::TryFromPrimitive;

#[derive(Copy, Clone, TryFromPrimitive, PartialEq, Eq, Debug)]
#[repr(u8)]
pub enum SelectOptions {
    Absolute = 0,
    Relative = 1,
}

#[derive(Copy, Clone, TryFromPrimitive, PartialEq, Eq, Debug)]
#[repr(u8)]
pub enum RangeOptions {
    AbsoluteStart = 0,
    RelativeStart = 1,
    AbsoluteEnd = 2,
    RelativeEnd = 3,
}

#[derive(Copy, Clone, TryFromPrimitive, PartialEq, Eq, Debug)]
#[repr(u8)]
pub enum ColorOptions {
    Red = 0,
    Green = 1,
    Blue = 2,
}

#[derive(Copy, Clone, TryFromPrimitive, PartialEq, Eq, Debug)]
#[repr(u8)]
pub enum EffectOptions {
    ApplyAll = 0,
    ApplyRange = 1,
}

#[derive(Copy, Clone, TryFromPrimitive, PartialEq, Eq, Debug)]
#[repr(u8)]
pub enum TimeOptions {
    Ms = 1,
    Sec = 2,
    Min = 3,
}

#[derive(Copy, Clone, TryFromPrimitive, PartialEq, Eq, Debug)]
#[repr(u8)]
pub enum LoadOptions {
    Variable = 0,
    Red = 1,
    Green = 2,
    Blue = 3,
}

#[derive(Copy, Clone, PartialEq, Eq, Debug)]
#[repr(u8)]
pub enum InstructionOptions {
    None,
    Select(SelectOptions),
    Range(RangeOptions),
    Color(ColorOptions),
    Effect(EffectOptions),
    Time(TimeOptions),
    Load(LoadOptions),
}
