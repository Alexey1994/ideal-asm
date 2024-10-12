typedef struct {
	Byte*   name;
	Number  address;
	Boolean undefined_address;
}
Link;


Dynamic_Stack links;


Link* find_link(Byte* name)
{
	Number i;
	Link*  link;

	for(i = 0; i < links.stack.size; i += sizeof(Link*)) {
		link = *(Link**)(links.stack.data + i);

		if(!compare_strings(link->name, name)) {
			return link;
		}
	}

	return 0;
}


Link* create_link(Byte* name)
{
	if(find_link(name)) {
		return 0;
	}

	Link* link;

	link = allocate_memory(sizeof(Link));
	link->name = name;
	link->address = 0xFFFFFFFF;
	link->undefined_address = 1;

	write_bytes_in_dynamic_stack(&links, &link, sizeof(link));

	return link;
}
