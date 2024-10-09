#include <memory.c>


Number        line_number = 1;
Number32      const_number_value;
Real_Number32 const_real_number_value;
Dynamic_Stack name_value;


void error(Number line_number, Byte* parameters, ...)
{
	write_Number(&stderr, line_number);
	write_string(&stderr, ": ");
	write(&stderr, parameters, &parameters + 1);
	write_string(&stderr, "\n");

	ExitProcess(1);
}


typedef enum {
	EOF_TOKEN = 256,

	NUMBER_TOKEN,
	LINK_TOKEN,
	NAME_TOKEN,

	BYTE_TOKEN,
	WORD_TOKEN,
	DWORD_TOKEN,

	ORG_TOKEN,
	DB_TOKEN,

	AL_TOKEN,
	CL_TOKEN,
	DL_TOKEN,
	BL_TOKEN,
	AH_TOKEN,
	CH_TOKEN,
	DH_TOKEN,
	BH_TOKEN,

	AX_TOKEN,
	CX_TOKEN,
	DX_TOKEN,
	BX_TOKEN,
	SP_TOKEN,
	BP_TOKEN,
	SI_TOKEN,
	DI_TOKEN,

	ES_TOKEN,
	CS_TOKEN,
	SS_TOKEN,
	DS_TOKEN,

	ADD_TOKEN,
	OR_TOKEN,
	ADC_TOKEN,
	SBB_TOKEN,
	AND_TOKEN,
	SUB_TOKEN,
	XOR_TOKEN,
	CMP_TOKEN,

	INC_TOKEN,
	DEC_TOKEN,
	CALL_TOKEN,
	CALLF_TOKEN,
	JMP_TOKEN,
	JMPF_TOKEN,
	PUSH_TOKEN,
	POP_TOKEN,

	JO_TOKEN,
	JNO_TOKEN,
	JC_TOKEN,
	JNC_TOKEN,
	JZ_TOKEN,
	JNZ_TOKEN,
	JNA_TOKEN,
	JA_TOKEN,
	JS_TOKEN,
	JNS_TOKEN,
	JPE_TOKEN,
	JPO_TOKEN,
	JL_TOKEN,
	JNL_TOKEN,
	JNG_TOKEN,
	JG_TOKEN,

	TEST_TOKEN,
	XCHG_TOKEN,
	MOV_TOKEN,
	LEA_TOKEN,

	NOT_TOKEN,
	NEG_TOKEN,
	MUL_TOKEN,
	IMUL_TOKEN,
	DIV_TOKEN,
	IDIV_TOKEN,

	ROL_TOKEN,
	ROR_TOKEN,
	RCL_TOKEN,
	RCR_TOKEN,
	SHL_TOKEN,
	SHR_TOKEN,
	SAL_TOKEN,
	SAR_TOKEN,

	DAA_TOKEN,
	DAS_TOKEN,
	AAA_TOKEN,
	AAS_TOKEN,

	CBW_TOKEN,
	CWD_TOKEN,

	WAIT_TOKEN,

	PUSHF_TOKEN,
	POPF_TOKEN,
	SAHF_TOKEN,
	LAHF_TOKEN,

	MOVSB_TOKEN,
	MOVSW_TOKEN,
	CMPSB_TOKEN,
	CMPSW_TOKEN,
	STOSB_TOKEN,
	STOSW_TOKEN,
	LODSB_TOKEN,
	LODSW_TOKEN,
	SCASB_TOKEN,
	SCASW_TOKEN,

	RETN_TOKEN,
	RET_TOKEN,
	RETFN_TOKEN,
	RETF_TOKEN,

	INT3_TOKEN,
	INT_TOKEN,
	INTO_TOKEN,
	IRET_TOKEN,

	LOOPNZ_TOKEN,
	LOOPZ_TOKEN,
	LOOP_TOKEN,
	JCXZ_TOKEN,

	IN_TOKEN,
	OUT_TOKEN,

	LOCK_TOKEN,
	INT1_TOKEN,
	REPNE_TOKEN,
	REP_TOKEN,
	HLT_TOKEN,
	CMC_TOKEN,
	CLC_TOKEN,
	STC_TOKEN,
	CLI_TOKEN,
	STI_TOKEN,
	CLD_TOKEN,
	STD_TOKEN,

	PUSHA_TOKEN,
	POPA_TOKEN,
	
	BOUND_TOKEN,

	INSB_TOKEN,
	INSW_TOKEN,
	OUTSB_TOKEN,
	OUTSW_TOKEN,

	ENTER_TOKEN,
	LEAVE_TOKEN,




	REG8_TOKEN,
	REG16_TOKEN,
	SEG_TOKEN,
	MEM_TOKEN,
	NEG_OPERATION_TOKEN,

	LONG_ADDRESS_TOKEN,
	CALCULATED_NUMBER_TOKEN,
}
Token;


Byte token_names[][16] = {
	"<EOF>",

	"number",
	"link",
	"name",

	"byte",
	"word",
	"dword",

	"org",
	"db",

	"AL",
	"CL",
	"DL",
	"BL",
	"AH",
	"CH",
	"DH",
	"BH",

	"AX",
	"CX",
	"DX",
	"BX",
	"SP",
	"BP",
	"SI",
	"DI",

	"ES",
	"CS",
	"SS",
	"DS",

	"add",
	"or",
	"adc",
	"sbb",
	"and",
	"sub",
	"xor",
	"cmp",

	"inc",
	"dec",
	"call",
	"callf",
	"jmp",
	"jmpf",
	"push",
	"pop",

	"jo",
	"jno",
	"jc",
	"jnc",
	"jz",
	"jnz",
	"jna",
	"ja",
	"js",
	"jns",
	"jpe",
	"jpo",
	"jl",
	"jnl",
	"jng",
	"jg",

	"test",
	"xchg",
	"mov",
	"lea",

	"not",
	"neg",
	"mul",
	"imul",
	"div",
	"idiv",

	"rol",
	"ror",
	"rcl",
	"rcr",
	"shl",
	"shr",
	"sal",
	"sar",

	"daa",
	"das",
	"aaa",
	"aas",

	"cbw",
	"cwd",

	"wait",

	"pushf",
	"popf",
	"sahf",
	"lahf",

	"movsb",
	"movsw",
	"cmpsb",
	"cmpsw",
	"stosb",
	"stosw",
	"lodsb",
	"lodsw",
	"scasb",
	"scasw",

	"retn",
	"ret",
	"retfn",
	"retf",

	"int3",
	"int",
	"into",
	"iret",

	"loopnz",
	"loopz",
	"loop",
	"jcxz",

	"in",
	"out",

	"lock",
	"int1",
	"repne",
	"rep",
	"hlt",
	"cmc",
	"clc",
	"stc",
	"cli",
	"sti",
	"cld",
	"std",

	"pusha",
	"popa",
	
	"bound",

	"insb",
	"insw",
	"outsb",
	"outsw",

	"enter",
	"leave",
};

/*
void print_token(Token token)
{
	if(token < 256) {
		print_error("%c", token);
	}
	else {
		if(token == NAME_TOKEN) {
			print_error("(%s)", name_value.stack.data);
		}
		else {
			print_error("%s", token_names[token - 256]);
		}
	}
}*/


Token read_next_token()
{
	Byte    head_character;
	Number  i;

	repeat: {
		if(peek_bytes(&stdin, &head_character, sizeof(head_character)) != sizeof(head_character)) {
			return EOF_TOKEN;
		}

		switch(head_character) {
			case '\n':
				++line_number;
			case '\r':
			case '\t':
			case ' ': {
				next(&stdin, 1);
				goto repeat;
			}

			case '#':
			case ';': {
				do {
					next(&stdin, 1);

					if(peek_bytes(&stdin, &head_character, sizeof(head_character)) != sizeof(head_character)) {
						return EOF_TOKEN;
					}
				}
				while(head_character != '\n');

				++line_number;
				next(&stdin, 1);

				goto repeat;
			}

			case '0'...'9': {
				const_number_value = 0;

				if(head_character == '0') {
					next(&stdin, 1);

					if(peek_bytes(&stdin, &head_character, sizeof(head_character)) != sizeof(head_character)) {
						return NUMBER_TOKEN;
					}

					if(head_character == 'x') {
						next(&stdin, 1);

						for(;;) {
							if(peek_bytes(&stdin, &head_character, sizeof(head_character)) != sizeof(head_character)) {
								break;
							}

							if(head_character >= '0' && head_character <= '9') {
								const_number_value = const_number_value * 16 + (head_character - '0');
							}
							else if(head_character >= 'A' && head_character <= 'F') {
								const_number_value = const_number_value * 16 + (head_character - 'A' + 10);
							}
							else {
								break;
							}

							next(&stdin, 1);
						}
					}
				}
				else {
					for(;;) {
						if(peek_bytes(&stdin, &head_character, sizeof(head_character)) != sizeof(head_character)) {
							break;
						}

						if(!(head_character >= '0' && head_character <= '9')) {
							break;
						}

						const_number_value = const_number_value * 10 + (head_character - '0');
						next(&stdin, 1);
					}
				}

				return NUMBER_TOKEN;
			}

			case '\'': {
				const_number_value = 0;

				for(;;) {
					next(&stdin, 1);

					if(peek_bytes(&stdin, &head_character, sizeof(head_character)) != sizeof(head_character)) {
						error(line_number, "end in '");
					}

					if(head_character == '\'') {
						next(&stdin, 1);
						break;
					}
					else if(head_character == '\\') {
						next(&stdin, 1);

						if(peek_bytes(&stdin, &head_character, sizeof(head_character)) != sizeof(head_character)) {
							error(line_number, "end in '");
						}
						
						switch(head_character) {
							case 'n': {
								head_character = '\n';
								break;
							}
							
							case 'r': {
								head_character = '\r';
								break;
							}
							
							case 't': {
								head_character = '\t';
								break;
							}
							
							case 'b': {
								head_character = '\b';
								break;
							}
						}
					}
					
					const_number_value = head_character;
				}

				return NUMBER_TOKEN;
			}

			case 'a'...'z':
			case 'A'...'Z':
			case '_': {
				initialize_dynamic_stack(&name_value, 32);

				do {
					write_bytes_in_dynamic_stack(&name_value, &head_character, 1);
					next(&stdin, 1);

					if(peek_bytes(&stdin, &head_character, sizeof(head_character)) != sizeof(head_character)) {
						break;
					}
				}
				while(
					head_character >= 'a' && head_character <= 'z'
					|| head_character >= 'A' && head_character <= 'Z'
					|| head_character >= '0' && head_character <= '9'
					|| head_character == '_'
				);
				head_character = '\0';
				write_bytes_in_dynamic_stack(&name_value, &head_character, 1);

				for(i = BYTE_TOKEN; i <= LEAVE_TOKEN; ++i) {
					if(!compare_strings(token_names[i - EOF_TOKEN], name_value.stack.data)) {
						deinitialize_dynamic_stack(&name_value);
						return i;
					}
				}

				if(peek_bytes(&stdin, &head_character, sizeof(head_character)) == sizeof(head_character) && head_character == ':') {
					next(&stdin, 1);
					return LINK_TOKEN;
				}

				return NAME_TOKEN;
			}

			default: {
				next(&stdin, 1);
				
				return head_character;
			}
		}
	}
}