org 0x7C00

mov SI message
call print_string

end: hlt jmp end

#in  SI - printed string
print_string:
	mov AH 0x0E
print_next_string_char:
	lodsb
	cmp AL 0
	jz end_print_string
	int 0x10
	jmp print_next_string_char
end_print_string:
	ret

message: 'H' 'i' 0