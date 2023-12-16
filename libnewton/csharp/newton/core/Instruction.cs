namespace Prism.Newton.Core;

public class Instruction(InstructionSet instruction, InstructionOptions options, byte value)
{
    public InstructionSet InstructionType { get; set; } = instruction;
    public InstructionOptions Options { get; set; } = options;
    public byte Value { get; set; } = value;

    public Instruction() : this(InstructionSet.Ignore, new InstructionOptions.None(), 0x00)
    {
    }

    public static Instruction ParseFromU16(ushort bytes)
    {
        // Parse instruction from raw values
        var rawInstruction = new
        Newton.Core.RawInstruction
        {
            Instruction = (byte)((bytes & 0xFC00) >> 10),
            Options = (byte)((bytes & 0x0300) >> 8),
            Value = (byte)(bytes & 0x00FF)
        };

        // Parse option from byte
        var instruction = new Instruction(
                (Newton.Core.InstructionSet)rawInstruction.Instruction,
                new InstructionOptions.None(),
                rawInstruction.Value
            );

        // Match the option
        switch (instruction.InstructionType)
        {
            case InstructionSet.Clear:
            case InstructionSet.Update:
            case InstructionSet.Ignore:
                break;

            case InstructionSet.Sleep:
                instruction.Options = new Newton.Core.InstructionOptions.Time(
                            (Newton.Core.TimeOptions)rawInstruction.Options
                        );
                break;

            case InstructionSet.Select:
                instruction.Options = new Newton.Core.InstructionOptions.Select(
                            (Newton.Core.SelectOptions)rawInstruction.Options
                        );
                break;

            case InstructionSet.Range:
                instruction.Options = new Newton.Core.InstructionOptions.Range(
                            (Newton.Core.RangeOptions)rawInstruction.Options
                        );
                break;

            case InstructionSet.Set:
                instruction.Options = new Newton.Core.InstructionOptions.Color(
                            (Newton.Core.ColorOptions)rawInstruction.Options
                        );
                break;

            case InstructionSet.Fill:
                instruction.Options = new Newton.Core.InstructionOptions.Color(
                            (Newton.Core.ColorOptions)rawInstruction.Options
                        );
                break;

            case InstructionSet.Blur:
                instruction.Options = new Newton.Core.InstructionOptions.Effect(
                            (Newton.Core.EffectOptions)rawInstruction.Options
                        );
                break;

            case InstructionSet.LoadX:
                instruction.Options = new Newton.Core.InstructionOptions.Load(
                            (Newton.Core.LoadOptions)rawInstruction.Options
                        );
                break;

            case InstructionSet.LoadY:
                instruction.Options = new Newton.Core.InstructionOptions.Load(
                            (Newton.Core.LoadOptions)rawInstruction.Options
                        );
                break;

            case InstructionSet.Exception:
            default:
                throw new Newton.Error.InvalidInstructionError(rawInstruction);
        }

        return instruction;
    }

    public ushort WriteToU16()
    {
        byte rawOptions = 0x00;

        if (this.Options is InstructionOptions.None)
            rawOptions = 0x00;
        else if (this.Options is InstructionOptions.Color)
            rawOptions = (byte)(this.Options as InstructionOptions.Color)!.options;
        else if (this.Options is InstructionOptions.Effect)
            rawOptions = (byte)(this.Options as InstructionOptions.Effect)!.options;
        else if (this.Options is InstructionOptions.Load)
            rawOptions = (byte)(this.Options as InstructionOptions.Load)!.options;
        else if (this.Options is InstructionOptions.Range)
            rawOptions = (byte)(this.Options as InstructionOptions.Range)!.options;
        else if (this.Options is InstructionOptions.Select)
            rawOptions = (byte)(this.Options as InstructionOptions.Select)!.options;
        else if (this.Options is InstructionOptions.Time)
            rawOptions = (byte)(this.Options as InstructionOptions.Time)!.options;
        else
            throw new Newton.Error.InvalidOptionForInstruction(this.InstructionType, this.Options);

        var rawInstruction = new Newton.Core.RawInstruction
        {
            Instruction = (byte)this.InstructionType,
            Options = rawOptions,
            Value = this.Value,
        };

        ushort bytes = 0x0000;
        bytes |= (ushort)((ushort)rawInstruction.Options << 8);
        bytes |= (ushort)(((ushort)rawInstruction.Instruction << 10) & 0xFF00);
        bytes |= (ushort)((ushort)rawInstruction.Value & 0x00FF);

        return bytes;
    }
}
