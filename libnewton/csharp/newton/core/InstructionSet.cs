namespace Prism.Newton.Core;

public enum InstructionSet : byte
{
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
