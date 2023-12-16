namespace Prism.Newton.Core;

public enum SelectOptions : byte
{
    Absolute = 0,
    Relative = 1,
}

public enum RangeOptions : byte
{
    AbsoluteStart = 0,
    RelativeStart = 1,
    AbsoluteEnd = 2,
    RelativeEnd = 3,
}

public enum ColorOptions : byte
{
    Red = 0,
    Green = 1,
    Blue = 2,
}

public enum EffectOptions : byte
{
    ApplyAll = 0,
    ApplyRange = 1,
}

public enum TimeOptions : byte
{
    Ms = 1,
    Sec = 2,
    Min = 3,
}

public enum LoadOptions : byte
{
    Variable = 0,
    Red = 1,
    Green = 2,
    Blue = 3,
}

public abstract class InstructionOptions
{
    public sealed class None : InstructionOptions { }

    public sealed class Select(SelectOptions options) : InstructionOptions
    {
        public SelectOptions options { get; } = options;
    }

    public sealed class Range(RangeOptions options) : InstructionOptions
    {
        public RangeOptions options { get; } = options;
    }

    public sealed class Color(ColorOptions options) : InstructionOptions
    {
        public ColorOptions options { get; } = options;
    }

    public sealed class Effect(EffectOptions options) : InstructionOptions
    {
        public EffectOptions options { get; } = options;
    }

    public sealed class Time(TimeOptions options) : InstructionOptions
    {
        public TimeOptions options { get; } = options;
    }

    public sealed class Load(LoadOptions options) : InstructionOptions
    {
        public LoadOptions options { get; } = options;
    }
}
