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
	args: [files('../../../examples/files/halfs.prism'), files('../../../examples/files/halfs.bin')],
)

test(
	'Load variables into registers',
	executable(
		'RegisterVariableLoading',
		files('register_variable_loading.c'),
		c_args: ['-DDEBUG'],
		include_directories: [include],
	),
	args: [files('../../../examples/files/registers.bin')],
)

