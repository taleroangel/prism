project(
	'Prism',
	'c',
	version: '0.1',
	default_options: ['warning_level=3', 'c_std=c2x'],
)

include = include_directories('include')

subdir('test')

# Newton Prism compiler
executable(
	'NewtonCompiler',
	files('tools/newton_compiler.c'),
	include_directories: [include],
)
