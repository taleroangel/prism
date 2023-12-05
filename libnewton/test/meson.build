test(
	'Instruction parsing from U16 to PrismInstruction',
	executable(
		'InstructionParsingFromU16',
		files('instruction_parsing_from_u16.c'),
		c_args: ['-DDEBUG', '-Wno-gnu-binary-literal'],
		include_directories: [include],
	),
)

test(
	'Instruction parsing from String literal to PrismInstruction',
	executable(
		'InstructionParsingFromLiteral',
		files('instruction_parsing_from_literal.c'),
		c_args: ['-DDEBUG'],
		include_directories: [include],
	),
	args: [files('files/instructions.prism'), files('files/instructions.bin')],
)

