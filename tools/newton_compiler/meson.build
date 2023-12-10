project(
	'newton_compiler',
	'c',
	version: '0.1',
	default_options: ['warning_level=3', 'c_std=c2x'],
)

include = [include_directories('../../libnewton/c/include')]

executable(
	'newton_compiler',
	'newton_compiler.c',
	include_directories: include,
	install: true,
)
