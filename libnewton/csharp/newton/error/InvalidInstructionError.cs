namespace Prism.Newton.Error;

public sealed class InvalidInstructionError(Prism.Newton.Core.RawInstruction rawInstruction) : NewtonError
{
    public Prism.Newton.Core.RawInstruction RawInstruction { get; } = rawInstruction;
}
