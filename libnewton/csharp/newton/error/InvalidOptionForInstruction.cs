namespace Prism.Newton.Error;

public class InvalidOptionForInstruction(
      Newton.Core.InstructionSet instruction,
      Newton.Core.InstructionOptions option
    ) : NewtonError
{
    public Newton.Core.InstructionSet Instruction { get; } = instruction;
    public Newton.Core.InstructionOptions Option { get; } = option;
}
