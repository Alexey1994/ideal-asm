typedef struct {
	Token  token;
	Number line_number;
}
Node;


typedef struct {
	Token  token;
	Number line_number;
	Link*  link;
}
Link_Node;

typedef struct {
	Token    token;
	Number   line_number;
	Number32 value;
	Number   size;
}
Number_Node;

typedef struct {
	Token    token;
	Number   line_number;
	Number16 segment;
	Node*    offset;
}
Long_Address_Node;

typedef struct {
	Token  token;
	Number line_number;
	Byte*  link_name;
	Link*  link;
}
Name_Link_Node;

typedef struct {
	Token    token;
	Number   line_number;
	Number32 index;
}
Reg8_Node;

typedef struct {
	Token    token;
	Number   line_number;
	Number32 index;
}
Reg16_Node;

typedef struct {
	Token    token;
	Number   line_number;
	Number32 index;
}
Reg32_Node;

typedef struct {
	Token    token;
	Number   line_number;
	Number32 index;
}
Seg_Node;

typedef struct {
	Token    token;
	Number   line_number;
	Number32 index;
}
CRx_Node;

typedef struct {
	Token    token;
	Number   line_number;
	Number32 index;
}
DRx_Node;

typedef struct {
	Token    token;
	Number   line_number;
	Number32 index;
}
TRx_Node;

typedef struct {
	Token        token;
	Number       line_number;
	Node*        expression;
	Number       size;
	Number       segment;
	Reg16_Node*  first_reg;
	Reg16_Node*  second_reg;
	Number_Node* offset;
	Number_Node* first_reg_shift;
	Number_Node* second_reg_shift;
	Number       bitness;
}
Mem_Node;

typedef struct {
	Token    token;
	Number   line_number;
	Node*    operand;
}
Unary_Operation_Node;

typedef struct {
	Token    token;
	Number   line_number;
	Node*    left_operand;
	Node*    right_operand;
}
Binary_Operation_Node;

typedef struct {
	Token    token;
	Number   line_number;
	Node*    left_operand;
	Node*    center_operand;
	Node*    right_operand;
}
Ternary_Operation_Node;

typedef struct {
	Token    token;
	Number   line_number;
	Node*    left_operand;
	Node*    destination_operand;
	Node*    right_operand;
}
Imul_Node;


Token token;


Node* create_node(Number size)
{
	Node* node = allocate_memory(size);

	node->token = token;
	node->line_number = line_number;

	return node;
}


Node* parse_expression();


Node* parse_operand()
{
	Number operand_size;
	Number segment_index;

	operand_size = 0;
	segment_index = 0xFF;

	if(token == ',') {
		token = read_next_token();
	}

	switch(token) {
		case BYTE_TOKEN: {
			operand_size = 8;
			token = read_next_token();
			break;
		}

		case WORD_TOKEN: {
			operand_size = 16;
			token = read_next_token();
			break;
		}

		case DWORD_TOKEN: {
			operand_size = 32;
			token = read_next_token();
			break;
		}
	}

	switch(token) {
		case ES_TOKEN:
		case CS_TOKEN:
		case SS_TOKEN:
		case DS_TOKEN:
		case FS_TOKEN:
		case GS_TOKEN: {
			Number l_n = line_number;

			segment_index = token - ES_TOKEN;
			token = read_next_token();

			if(token == ':') {
				token = read_next_token();
			}
			else {
				Seg_Node* new_node = create_node(sizeof(Seg_Node));
				new_node->token = SEG_TOKEN;
				new_node->line_number = l_n;
				new_node->index = segment_index;

				return new_node;
			}

			break;
		}
	}

	switch(token) {
		case NUMBER_TOKEN: {
			Number l_n = line_number;
			Number value = const_number_value;
			token = read_next_token();

			if(token == ':') {
				Long_Address_Node* new_node = create_node(sizeof(Long_Address_Node));
				new_node->token = LONG_ADDRESS_TOKEN;
				new_node->line_number = l_n;
				new_node->segment = const_number_value;

				token = read_next_token();

				new_node->offset = parse_expression();

				return new_node;
			}
			else {
				Number_Node* new_node = create_node(sizeof(Number_Node));
				new_node->token = NUMBER_TOKEN;
				new_node->line_number = l_n;
				new_node->value = value;
				new_node->size = operand_size;

				return new_node;
			}
		}

		case NAME_TOKEN: {
			Name_Link_Node* new_node = create_node(sizeof(Name_Link_Node));
			new_node->link_name = name_value.stack.data;
			new_node->link = 0;

			token = read_next_token();

			return new_node;
		}

		case AL_TOKEN:
		case CL_TOKEN:
		case DL_TOKEN:
		case BL_TOKEN:
		case AH_TOKEN:
		case CH_TOKEN:
		case DH_TOKEN:
		case BH_TOKEN: {
			Reg8_Node* new_node = create_node(sizeof(Reg8_Node));
			new_node->token = REG8_TOKEN;
			new_node->index = token - AL_TOKEN;

			token = read_next_token();

			return new_node;
		}

		case AX_TOKEN:
		case CX_TOKEN:
		case DX_TOKEN:
		case BX_TOKEN:
		case SP_TOKEN:
		case BP_TOKEN:
		case SI_TOKEN:
		case DI_TOKEN: {
			Reg16_Node* new_node = create_node(sizeof(Reg16_Node));
			new_node->token = REG16_TOKEN;
			new_node->index = token - AX_TOKEN;

			token = read_next_token();

			return new_node;
		}

		case EAX_TOKEN:
		case ECX_TOKEN:
		case EDX_TOKEN:
		case EBX_TOKEN:
		case ESP_TOKEN:
		case EBP_TOKEN:
		case ESI_TOKEN:
		case EDI_TOKEN: {
			Reg32_Node* new_node = create_node(sizeof(Reg32_Node));
			new_node->token = REG32_TOKEN;
			new_node->index = token - EAX_TOKEN;

			token = read_next_token();

			return new_node;
		}

		case CR0_TOKEN:
		case CR1_TOKEN:
		case CR2_TOKEN:
		case CR3_TOKEN:
		case CR4_TOKEN:
		case CR5_TOKEN:
		case CR6_TOKEN:
		case CR7_TOKEN: {
			Reg32_Node* new_node = create_node(sizeof(Reg32_Node));
			new_node->token = CRx_TOKEN;
			new_node->index = token - CR0_TOKEN;

			token = read_next_token();

			return new_node;
		}

		case DR0_TOKEN:
		case DR1_TOKEN:
		case DR2_TOKEN:
		case DR3_TOKEN:
		case DR4_TOKEN:
		case DR5_TOKEN:
		case DR6_TOKEN:
		case DR7_TOKEN: {
			Reg32_Node* new_node = create_node(sizeof(Reg32_Node));
			new_node->token = DRx_TOKEN;
			new_node->index = token - DR0_TOKEN;

			token = read_next_token();

			return new_node;
		}

		case TR0_TOKEN:
		case TR1_TOKEN:
		case TR2_TOKEN:
		case TR3_TOKEN:
		case TR4_TOKEN:
		case TR5_TOKEN:
		case TR6_TOKEN:
		case TR7_TOKEN: {
			Reg32_Node* new_node = create_node(sizeof(Reg32_Node));
			new_node->token = TRx_TOKEN;
			new_node->index = token - TR0_TOKEN;

			token = read_next_token();

			return new_node;
		}

		case '[': {
			Mem_Node* new_node = create_node(sizeof(Mem_Node));
			new_node->token = MEM_TOKEN;
			new_node->size = operand_size;
			new_node->segment = segment_index;
			new_node->first_reg = 0;
			new_node->second_reg = 0;
			new_node->offset = 0;
			new_node->first_reg_shift = 0;
			new_node->second_reg_shift = 0;
			new_node->bitness = 0;

			token = read_next_token();

			if(token == WORD_TOKEN) {
				new_node->bitness = 16;
				token = read_next_token();
			}
			else if(token == DWORD_TOKEN) {
				new_node->bitness = 32;
				token = read_next_token();
			}

			new_node->expression = parse_expression();

			if(token != ']') {
				error(line_number, "expected ]");
			}

			token = read_next_token();

			return new_node;
		}

		case '-': {
			Unary_Operation_Node* new_node = create_node(sizeof(Unary_Operation_Node));
			new_node->token = NEG_OPERATION_TOKEN;

			token = read_next_token();

			new_node->operand = parse_operand();

			return new_node;
		}

		case '(': {
			token = read_next_token();

			Node* new_node = parse_expression();
			
			if(token != ')') {
				error(line_number, "expected )");
			}
			
			token = read_next_token();

			return new_node;
		}

		default: {
			error(line_number, "expected operand");
		}
	}
}


Node* parse_arithmetic_expression()
{
	Node* new_node;

	new_node = parse_operand();

	while(
		token == '*'
		|| token == '/'
		|| token == SHIFT_LEFT_TOKEN
	) {
		Binary_Operation_Node* operation_node = create_node(sizeof(Binary_Operation_Node));
		operation_node->left_operand = new_node;
		
		token = read_next_token();

		operation_node->right_operand = parse_operand();

		new_node = operation_node;
	}

	return new_node;
}


Node* parse_expression()
{
	Node* new_node;

	new_node = parse_arithmetic_expression();

	while(
		token == '+'
		|| token == '-'
	) {
		Binary_Operation_Node* operation_node = create_node(sizeof(Binary_Operation_Node));
		operation_node->left_operand = new_node;
		
		token = read_next_token();

		operation_node->right_operand = parse_arithmetic_expression();

		new_node = operation_node;
	}

	return new_node;
}


Node* parse_instruction()
{
	switch(token) {
		case LINK_TOKEN: {
			Link_Node* new_node = create_node(sizeof(Link_Node));
			new_node->link = create_link(name_value.stack.data);

			if(!new_node->link) {
				error(line_number, "link %s is defined", name_value.stack.data);
			}

			token = read_next_token();
			return new_node;
		}

		case ADD_TOKEN:
		case OR_TOKEN:
		case ADC_TOKEN:
		case SBB_TOKEN:
		case AND_TOKEN:
		case SUB_TOKEN:
		case XOR_TOKEN:
		case CMP_TOKEN:

		case TEST_TOKEN:
		case XCHG_TOKEN:
		case MOV_TOKEN:
		case LEA_TOKEN:
		case LES_TOKEN:
		case LDS_TOKEN:
		case LSS_TOKEN:
		case LFS_TOKEN:
		case LGS_TOKEN:

		case ROL_TOKEN:
		case ROR_TOKEN:
		case RCL_TOKEN:
		case RCR_TOKEN:
		case SHL_TOKEN:
		case SHR_TOKEN:
		case SAL_TOKEN:
		case SAR_TOKEN:

		case IN_TOKEN:
		case OUT_TOKEN:

		case BOUND_TOKEN:

		case ENTER_TOKEN:

		case ARPL_TOKEN:

		case LAR_TOKEN:
		case LSL_TOKEN:

		case BT_TOKEN:
		case BTS_TOKEN:
		case BTR_TOKEN:
		case BTC_TOKEN:

		case MOVZX_TOKEN:
		case MOVSX_TOKEN:

		case BSF_TOKEN:
		case BSR_TOKEN: {
			Binary_Operation_Node* new_node = create_node(sizeof(Binary_Operation_Node));

			token = read_next_token();

			new_node->left_operand = parse_expression();

			if(token != ',') {
				error(line_number, "expected ,");
			}

			new_node->right_operand = parse_expression();

			return new_node;
		}

		case INC_TOKEN:
		case DEC_TOKEN:
		case CALL_TOKEN:
		case CALLF_TOKEN:
		case JMP_TOKEN:
		case JMPF_TOKEN:
		case PUSH_TOKEN:
		case POP_TOKEN:

		case JO_TOKEN:
		case JNO_TOKEN:
		case JC_TOKEN:
		case JNC_TOKEN:
		case JZ_TOKEN:
		case JNZ_TOKEN:
		case JNA_TOKEN:
		case JA_TOKEN:
		case JS_TOKEN:
		case JNS_TOKEN:
		case JPE_TOKEN:
		case JPO_TOKEN:
		case JL_TOKEN:
		case JNL_TOKEN:
		case JNG_TOKEN:
		case JG_TOKEN:

		case NOT_TOKEN:
		case NEG_TOKEN:
		case MUL_TOKEN:
		//case IMUL_TOKEN:
		case DIV_TOKEN:
		case IDIV_TOKEN:

		case RETN_TOKEN:
		case RETFN_TOKEN:

		case INT_TOKEN:

		case LOOPNZ_TOKEN:
		case LOOPZ_TOKEN:
		case LOOP_TOKEN:
		case JECXZ_TOKEN:
		case JCXZ_TOKEN:

		case SLDT_TOKEN:
		case STR_TOKEN:
		case LLDT_TOKEN:
		case LTR_TOKEN:
		case VERR_TOKEN:
		case VERW_TOKEN:

		case SGDT_TOKEN:
		case SIDT_TOKEN:
		case LGDT_TOKEN:
		case LIDT_TOKEN:
		case SMSW_TOKEN:
		case LMSW_TOKEN:

		case SETO_TOKEN:
		case SETNO_TOKEN:
		case SETC_TOKEN:
		case SETNC_TOKEN:
		case SETZ_TOKEN:
		case SETNZ_TOKEN:
		case SETNA_TOKEN:
		case SETA_TOKEN:
		case SETS_TOKEN:
		case SETNS_TOKEN:
		case SETPE_TOKEN:
		case SETPO_TOKEN:
		case SETL_TOKEN:
		case SETNL_TOKEN:
		case SETNG_TOKEN:
		case SETG_TOKEN:

		case ORG_TOKEN: {
			Unary_Operation_Node* new_node = create_node(sizeof(Unary_Operation_Node));

			token = read_next_token();

			new_node->operand = parse_expression();

			return new_node;
		}

		case DAA_TOKEN:
		case DAS_TOKEN:
		case AAA_TOKEN:
		case AAS_TOKEN:

		case CWDE_TOKEN:
		case CBW_TOKEN:
		case CDQ_TOKEN:
		case CWD_TOKEN:

		case WAIT_TOKEN:

		case PUSHF_TOKEN:
		case POPF_TOKEN:
		case SAHF_TOKEN:
		case LAHF_TOKEN:

		case MOVSB_TOKEN:
		case MOVSW_TOKEN:
		case MOVSD_TOKEN:
		case CMPSB_TOKEN:
		case CMPSW_TOKEN:
		case CMPSD_TOKEN:
		case STOSB_TOKEN:
		case STOSW_TOKEN:
		case STOSD_TOKEN:
		case LODSB_TOKEN:
		case LODSW_TOKEN:
		case LODSD_TOKEN:
		case SCASB_TOKEN:
		case SCASW_TOKEN:
		case SCASD_TOKEN:

		case RET_TOKEN:
		case RETF_TOKEN:

		case INT3_TOKEN:
		case INTO_TOKEN:
		case IRET_TOKEN:

		case LOCK_TOKEN:
		case INT1_TOKEN:
		case REPNE_TOKEN:
		case REP_TOKEN:
		case HLT_TOKEN:
		case CMC_TOKEN:
		case CLC_TOKEN:
		case STC_TOKEN:
		case CLI_TOKEN:
		case STI_TOKEN:
		case CLD_TOKEN:
		case STD_TOKEN:

		case PUSHA_TOKEN:
		case POPA_TOKEN:

		case INSB_TOKEN:
		case INSW_TOKEN:
		case INSD_TOKEN:
		case OUTSB_TOKEN:
		case OUTSW_TOKEN:
		case OUTSD_TOKEN:

		case LEAVE_TOKEN:

		case CLTS_TOKEN:

		case DB_TOKEN: {
			Node* new_node = create_node(sizeof(Node));
			
			token = read_next_token();

			return new_node;
		}

		case IMUL_TOKEN: {
			Imul_Node* new_node = create_node(sizeof(Imul_Node));

			token = read_next_token();

			new_node->left_operand = parse_expression();

			if(token == ',') {
				new_node->destination_operand = parse_expression();

				if(token == ',') {
					new_node->right_operand = parse_expression();
				}
			}
			else {
				new_node->destination_operand = 0;
				new_node->right_operand = 0;
			}

			return new_node;
		}

		case SHLD_TOKEN:
		case SHRD_TOKEN: {
			Ternary_Operation_Node* new_node = create_node(sizeof(Ternary_Operation_Node));

			token = read_next_token();

			new_node->left_operand = parse_expression();

			if(token != ',') {
				error(line_number, "expected ,");
			}

			new_node->center_operand = parse_expression();

			if(token != ',') {
				error(line_number, "expected ,");
			}

			new_node->right_operand = parse_expression();

			return new_node;
		}

		default: {
			return parse_expression();
		}
	}
}


void parse_program(Dynamic_Stack* program)
{
	Node* instruction;

	token = read_next_token();

	while(token != EOF_TOKEN) {
		instruction = parse_instruction();
		write_bytes_in_dynamic_stack(program, &instruction, sizeof(instruction));
	}
}