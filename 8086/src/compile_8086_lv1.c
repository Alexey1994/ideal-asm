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

	initialize_dynamic_stack(&links, 512);
	initialize_dynamic_stack(&program, 512);

	parse_program(&program);

	out = &out_number_of_bytes_in_current_address;
	generate(&program);

	Number current_size = current_address;

	for(;;) {
		generate(&program);
		
		if(current_address < current_size) {
			current_size = current_address;
			continue;
		}

		break;
	}

	out = &out_bytes_in_stdout;
	generate(&program);

	return 0;
}