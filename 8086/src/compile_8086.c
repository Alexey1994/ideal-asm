#include <types.c>
#include <data-structures/dynamic-stack.c>
#include <string.c>

#include "link.c"
#include "tokener.c"
#include "parser.c"
#include "generator.c"


Number main()
{
	Dynamic_Stack program;
	Number current_size;

	initialize_dynamic_stack(&links, 512);
	initialize_dynamic_stack(&program, 512);

	parse_program(&program);

	out = &out_number_of_bytes_in_current_address;
	generate(&program);

	do {
		current_size = current_address;
		generate(&program);
	}
	while(current_address < current_size);

	out = &out_bytes_in_stdout;
	generate(&program);

	return 0;
}