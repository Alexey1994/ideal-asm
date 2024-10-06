void (*out) (Number number_of_bytes, Byte first_byte, ...) = 0;
Number current_address;


void out_bytes_in_stdout(Number number_of_bytes, Byte first_byte, ...)
{
	Number* bytes;
	Number  i;

	current_address += number_of_bytes;
	bytes = &first_byte;

	for(i = 0; i < number_of_bytes; ++i) {
		write_Byte(&stdout, bytes[i]);
	}
}


void out_number_of_bytes_in_current_address(Number number_of_bytes, Byte first_byte, ...)
{
	current_address += number_of_bytes;
}


Number get_const_value_from_node(Node* node)
{
	switch(node->token) {
		case NUMBER_TOKEN: {
			Number_Node* n = node;

			return n->value;
		}

		case CALCULATED_NUMBER_TOKEN: {
			Number_Node* n = node;
			Number value;

			value = n->value;
			free_memory(n);

			return value;
		}

		default: {
			error(node->line_number, "can't get const value");
		}
	}
}


Node* calculate_Mem_Node(Mem_Node* node, Node* expression)
{
	switch(expression->token) {
		case NUMBER_TOKEN: {
			return expression;
		}

		case NAME_TOKEN: {
			Name_Link_Node* n = expression;

			if(!n->link) {
				n->link = find_link(n->link_name);
			}

			if(!n->link) {
				if(out == &out_bytes_in_stdout) {
					error(n->line_number, "link %s not found", n->link_name);
				}

				Number_Node* new_node = create_node(sizeof(Number_Node));
				new_node->token = CALCULATED_NUMBER_TOKEN;
				new_node->line_number = n->line_number;
				new_node->value = 0xFFFFFFFF; //TODO: better infinite definition

				return new_node;
			}

			Number_Node* new_node = create_node(sizeof(Number_Node));
			new_node->token = CALCULATED_NUMBER_TOKEN;
			new_node->line_number = n->line_number;
			new_node->value = n->link->address;

			return new_node;
		}

		case REG16_TOKEN: {
			Reg16_Node* n = expression;

			if(n->index != 3 && n->index != 5 && n->index != 6 && n->index != 7) {
				error(n->line_number, "only BX, BP, SI, DI registers supported in [...]");
			}

			if(node->first_reg) {
				if(node->second_reg) {
					error(expression->line_number, "too many regs inside [...]");
				}
				else {
					node->second_reg = n;
				}
			}
			else {
				node->first_reg = n;
			}

			return expression;
		}

		case NEG_OPERATION_TOKEN: {
			Unary_Operation_Node* n = expression;
			Node* operand;

			operand = calculate_Mem_Node(node, n->operand);
			Number value = get_const_value_from_node(operand);

			Number_Node* new_node = create_node(sizeof(Number_Node));
			new_node->token = CALCULATED_NUMBER_TOKEN;
			new_node->line_number = n->line_number;
			new_node->value = -value;

			return new_node;
		}

		case '+': {
			Binary_Operation_Node* n = expression;
			Node* left_operand;
			Node* right_operand;

			left_operand = calculate_Mem_Node(node, n->left_operand);
			right_operand = calculate_Mem_Node(node, n->right_operand);

			if(left_operand->token == REG16_TOKEN) {
				return right_operand;
			}

			Number left_value = get_const_value_from_node(left_operand);
			Number right_value = get_const_value_from_node(right_operand);

			Number_Node* new_node = create_node(sizeof(Number_Node));
			new_node->token = CALCULATED_NUMBER_TOKEN;
			new_node->line_number = n->line_number;
			new_node->value = left_value + right_value;

			return new_node;
		}

		case '-': {
			Binary_Operation_Node* n = expression;
			Node* left_operand;
			Node* right_operand;

			left_operand = calculate_Mem_Node(node, n->left_operand);
			right_operand = calculate_Mem_Node(node, n->right_operand);

			Number left_value = get_const_value_from_node(left_operand);
			Number right_value = get_const_value_from_node(right_operand);

			Number_Node* new_node = create_node(sizeof(Number_Node));
			new_node->token = CALCULATED_NUMBER_TOKEN;
			new_node->line_number = n->line_number;
			new_node->value = left_value - right_value;

			return new_node;
		}

		case '*': {
			Binary_Operation_Node* n = expression;
			Node* left_operand;
			Node* right_operand;

			left_operand = calculate_Mem_Node(node, n->left_operand);
			right_operand = calculate_Mem_Node(node, n->right_operand);

			Number left_value = get_const_value_from_node(left_operand);
			Number right_value = get_const_value_from_node(right_operand);

			Number_Node* new_node = create_node(sizeof(Number_Node));
			new_node->token = CALCULATED_NUMBER_TOKEN;
			new_node->line_number = n->line_number;
			new_node->value = left_value * right_value;

			return new_node;
		}

		case '/': {
			Binary_Operation_Node* n = expression;
			Node* left_operand;
			Node* right_operand;

			left_operand = calculate_Mem_Node(node, n->left_operand);
			right_operand = calculate_Mem_Node(node, n->right_operand);

			Number left_value = get_const_value_from_node(left_operand);
			Number right_value = get_const_value_from_node(right_operand);

			if(!right_value) {
				error(n->line_number, "division by zero");
			}

			Number_Node* new_node = create_node(sizeof(Number_Node));
			new_node->token = CALCULATED_NUMBER_TOKEN;
			new_node->line_number = n->line_number;
			new_node->value = left_value / right_value;

			return new_node;
		}

		default: {
			error(expression->line_number, "unsupported expression");
		}
	}
}


Node* calculate_expression(Node* expression)
{
	switch(expression->token) {
		case NUMBER_TOKEN:
		case LONG_ADDRESS_TOKEN:
		case REG8_TOKEN:
		case REG16_TOKEN:
		case SEG_TOKEN: {
			return expression;
		}

		case NAME_TOKEN: {
			Name_Link_Node* n = expression;

			if(!n->link) {
				n->link = find_link(n->link_name);
			}

			if(!n->link) {
				if(out == &out_bytes_in_stdout) {
					error(n->line_number, "link %s not found", n->link_name);
				}

				Number_Node* new_node = create_node(sizeof(Number_Node));
				new_node->token = CALCULATED_NUMBER_TOKEN;
				new_node->line_number = n->line_number;
				new_node->value = 0xFFFFFFFF; //TODO: better infinite definition

				return new_node;
			}

			Number_Node* new_node = create_node(sizeof(Number_Node));
			new_node->token = CALCULATED_NUMBER_TOKEN;
			new_node->line_number = n->line_number;
			new_node->value = n->link->address;

			return new_node;
		}

		case MEM_TOKEN: {
			Mem_Node* n = expression;

			Node* offset = calculate_Mem_Node(n, n->expression);

			switch(offset->token) {
				case NUMBER_TOKEN:
				case CALCULATED_NUMBER_TOKEN: {
					n->offset = offset;
					break;
				}
			}

			return expression;
		}

		case NEG_OPERATION_TOKEN: {
			Unary_Operation_Node* n = expression;
			Node* operand;

			operand = calculate_expression(n->operand);

			Number value = get_const_value_from_node(operand);

			Number_Node* new_node = create_node(sizeof(Number_Node));
			new_node->token = CALCULATED_NUMBER_TOKEN;
			new_node->line_number = n->line_number;
			new_node->value = -value;

			return new_node;
		}

		case '+': {
			Binary_Operation_Node* n = expression;
			Node* left_operand;
			Node* right_operand;

			left_operand = calculate_expression(n->left_operand);
			right_operand = calculate_expression(n->right_operand);

			Number left_value = get_const_value_from_node(left_operand);
			Number right_value = get_const_value_from_node(right_operand);

			Number_Node* new_node = create_node(sizeof(Number_Node));
			new_node->token = CALCULATED_NUMBER_TOKEN;
			new_node->line_number = n->line_number;
			new_node->value = left_value + right_value;

			return new_node;
		}

		case '-': {
			Binary_Operation_Node* n = expression;
			Node* left_operand;
			Node* right_operand;

			left_operand = calculate_expression(n->left_operand);
			right_operand = calculate_expression(n->right_operand);

			Number left_value = get_const_value_from_node(left_operand);
			Number right_value = get_const_value_from_node(right_operand);

			Number_Node* new_node = create_node(sizeof(Number_Node));
			new_node->token = CALCULATED_NUMBER_TOKEN;
			new_node->line_number = n->line_number;
			new_node->value = left_value - right_value;

			return new_node;
		}

		case '*': {
			Binary_Operation_Node* n = expression;
			Node* left_operand;
			Node* right_operand;

			left_operand = calculate_expression(n->left_operand);
			right_operand = calculate_expression(n->right_operand);

			Number left_value = get_const_value_from_node(left_operand);
			Number right_value = get_const_value_from_node(right_operand);

			Number_Node* new_node = create_node(sizeof(Number_Node));
			new_node->token = CALCULATED_NUMBER_TOKEN;
			new_node->line_number = n->line_number;
			new_node->value = left_value * right_value;

			return new_node;
		}

		case '/': {
			Binary_Operation_Node* n = expression;
			Node* left_operand;
			Node* right_operand;

			left_operand = calculate_expression(n->left_operand);
			right_operand = calculate_expression(n->right_operand);

			Number left_value = get_const_value_from_node(left_operand);
			Number right_value = get_const_value_from_node(right_operand);

			if(!right_value) {
				error(n->line_number, "division by zero");
			}

			Number_Node* new_node = create_node(sizeof(Number_Node));
			new_node->token = CALCULATED_NUMBER_TOKEN;
			new_node->line_number = n->line_number;
			new_node->value = left_value / right_value;

			return new_node;
		}
	}
}


void generate_mem(Mem_Node* mem_node, Byte function)
{
	Reg16_Node*  first_reg;
	Reg16_Node*  second_reg;
	Number_Node* offset;

	first_reg = mem_node->first_reg;
	second_reg = mem_node->second_reg;
	offset = mem_node->offset;

	if(first_reg) {
		if(second_reg) {
			if(offset) {
				if(offset->value >= -128 && (Signed_Number16)offset->value < 128) {
					switch(first_reg->index) {
						case 3: {
							switch(second_reg->index) {
								case 6: out(2, (1 << 6) | (function << 3) | 0, offset->value); break; //[BX+SI+offset8]
								case 7: out(2, (1 << 6) | (function << 3) | 1, offset->value); break; //[BX+DI+offset8]
								
								default: error(second_reg->line_number, "not supported addressing");
							}

							break;
						}

						case 5: {
							switch(second_reg->index) {
								case 6: out(2, (1 << 6) | (function << 3) | 2, offset->value); break; //[BP+SI+offset8]
								case 7: out(2, (1 << 6) | (function << 3) | 3, offset->value); break; //[BP+DI+offset8]
								
								default: error(second_reg->line_number, "not supported addressing");
							}

							break;
						}

						case 6: {
							switch(second_reg->index) {
								case 3: out(2, (1 << 6) | (function << 3) | 0, offset->value); break; //[BX+SI+offset8]
								case 5: out(2, (1 << 6) | (function << 3) | 2, offset->value); break; //[BP+SI+offset8]
								
								default: error(second_reg->line_number, "not supported addressing");
							}

							break;
						}

						case 7: {
							switch(second_reg->index) {
								case 3: out(2, (1 << 6) | (function << 3) | 1, offset->value); break; //[BX+DI+offset8]
								case 5: out(2, (1 << 6) | (function << 3) | 3, offset->value); break; //[BP+DI+offset8]
								
								default: error(second_reg->line_number, "not supported addressing");
							}

							break;
						}
					}
				}
				else {
					switch(first_reg->index) {
						case 3: {
							switch(second_reg->index) {
								case 6: out(3, (2 << 6) | (function << 3) | 0, offset->value, offset->value >> 8); break; //[BX+SI+offset16]
								case 7: out(3, (2 << 6) | (function << 3) | 1, offset->value, offset->value >> 8); break; //[BX+DI+offset16]
								
								default: error(second_reg->line_number, "not supported addressing");
							}

							break;
						}

						case 5: {
							switch(second_reg->index) {
								case 6: out(3, (2 << 6) | (function << 3) | 2, offset->value, offset->value >> 8); break; //[BP+SI+offset16]
								case 7: out(3, (2 << 6) | (function << 3) | 3, offset->value, offset->value >> 8); break; //[BP+DI+offset16]
								
								default: error(second_reg->line_number, "not supported addressing");
							}

							break;
						}

						case 6: {
							switch(second_reg->index) {
								case 3: out(3, (2 << 6) | (function << 3) | 0, offset->value, offset->value >> 8); break; //[BX+SI+offset16]
								case 5: out(3, (2 << 6) | (function << 3) | 2, offset->value, offset->value >> 8); break; //[BP+SI+offset16]
								
								default: error(second_reg->line_number, "not supported addressing");
							}

							break;
						}

						case 7: {
							switch(second_reg->index) {
								case 3: out(3, (2 << 6) | (function << 3) | 1, offset->value, offset->value >> 8); break; //[BX+DI+offset16]
								case 5: out(3, (2 << 6) | (function << 3) | 3, offset->value, offset->value >> 8); break; //[BP+DI+offset16]
								
								default: error(second_reg->line_number, "not supported addressing");
							}

							break;
						}
					}
				}
			}
			else {
				switch(first_reg->index) {
					case 3: {
						switch(second_reg->index) {
							case 6: out(1, (function << 3) | 0); break; //[BX+SI]
							case 7: out(1, (function << 3) | 1); break; //[BX+DI]
							
							default: error(second_reg->line_number, "not supported addressing");
						}

						break;
					}

					case 5: {
						switch(second_reg->index) {
							case 6: out(1, (function << 3) | 2); break; //[BP+SI]
							case 7: out(1, (function << 3) | 3); break; //[BP+DI]
							
							default: error(second_reg->line_number, "not supported addressing");
						}

						break;
					}

					case 6: {
						switch(second_reg->index) {
							case 3: out(1, (function << 3) | 0); break; //[BX+SI]
							case 5: out(1, (function << 3) | 2); break; //[BP+SI]
							
							default: error(second_reg->line_number, "not supported addressing");
						}

						break;
					}

					case 7: {
						switch(second_reg->index) {
							case 3: out(1, (function << 3) | 1); break; //[BX+DI]
							case 5: out(1, (function << 3) | 3); break; //[BP+DI]
							
							default: error(second_reg->line_number, "not supported addressing");
						}

						break;
					}
				}
			}
		}
		else {
			if(offset) {
				if(offset->value >= -128 && (Signed_Number16)offset->value < 128) {
					switch(first_reg->index) {
						case 3: out(2, (1 << 6) | (function << 3) | 7, offset->value); break; //[BX+offset8]
						case 5: out(2, (1 << 6) | (function << 3) | 6, offset->value); break; //[BP+offset8]
						case 6: out(2, (1 << 6) | (function << 3) | 4, offset->value); break; //[SI+offset8]
						case 7: out(2, (1 << 6) | (function << 3) | 5, offset->value); break; //[DI+offset8]
					}
				}
				else {
					switch(first_reg->index) {
						case 3: out(3, (2 << 6) | (function << 3) | 7, offset->value, offset->value >> 8); break; //[BX+offset16]
						case 5: out(3, (2 << 6) | (function << 3) | 6, offset->value, offset->value >> 8); break; //[BP+offset16]
						case 6: out(3, (2 << 6) | (function << 3) | 4, offset->value, offset->value >> 8); break; //[SI+offset16]
						case 7: out(3, (2 << 6) | (function << 3) | 5, offset->value, offset->value >> 8); break; //[DI+offset16]
					}
				}
			}
			else {
				switch(first_reg->index) {
					case 3: out(1, (function << 3) | 7); break; //[BX]
					case 5: out(2, (1 << 6) | (function << 3) | 6, 0x00); break; //[BP]
					case 6: out(1, (function << 3) | 4); break; //[SI]
					case 7: out(1, (function << 3) | 5); break; //[DI]
				}
			}
		}
	}
	else {
		out(3, (function << 3) | 6, offset->value, offset->value >> 8);
	}

	if(mem_node->offset && mem_node->offset->token == CALCULATED_NUMBER_TOKEN) {
		free_memory(mem_node->offset);
	}

	mem_node->first_reg = 0;
	mem_node->second_reg = 0;
	mem_node->offset = 0;
}


void generate_segment_prefix(Mem_Node* node)
{
	switch(node->segment) {
		case 0: {
			out(1, 0x26);
			break;
		}

		case 1: {
			out(1, 0x2E);
			break;
		}

		case 2: {
			out(1, 0x36);
			break;
		}

		case 3: {
			out(1, 0x3E);
			break;
		}
	}
}


void generate(Dynamic_Stack* program)
{
	Number i;
	Node*  node;

	current_address = 0;

	for(i = 0; i < program->stack.size; i += sizeof(Node*)) {
		node = *(Node**)(program->stack.data + i);

		switch(node->token) {
			case LINK_TOKEN: {
				if(out == &out_number_of_bytes_in_current_address) {
					Link_Node* n = node;

					n->link->address = current_address;
					n->link->undefined_address = 0;
				}

				break;
			}

			case ADD_TOKEN:
			case OR_TOKEN:
			case ADC_TOKEN:
			case SBB_TOKEN:
			case AND_TOKEN:
			case SUB_TOKEN:
			case XOR_TOKEN:
			case CMP_TOKEN: {
				Binary_Operation_Node* n = node;
				Node* calculated_left_operand = calculate_expression(n->left_operand);
				Node* calculated_right_operand = calculate_expression(n->right_operand);
				Number operation_index = node->token - ADD_TOKEN;

				switch(calculated_left_operand->token) {
					case REG8_TOKEN: {
						Reg8_Node* left_operand = calculated_left_operand;

						switch(calculated_right_operand->token) {
							case NUMBER_TOKEN:
							case CALCULATED_NUMBER_TOKEN: {
								Number_Node* right_operand = calculated_right_operand;

								if(left_operand->index == 0) {
									out(2, 0x04 + operation_index * 8, right_operand->value);
								}
								else {
									out(3, 0x80, (3 << 6) | (operation_index << 3) | left_operand->index, right_operand->value);
								}

								break;
							}

							case REG8_TOKEN: {
								Reg8_Node* right_operand = calculated_right_operand;

								out(2, 0x00 + operation_index * 8, (3 << 6) | (right_operand->index << 3) | left_operand->index);

								break;
							}

							case MEM_TOKEN: {
								generate_segment_prefix(calculated_right_operand);
								out(1, 0x3 + operation_index * 8);
								generate_mem(calculated_right_operand, left_operand->index);

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					case REG16_TOKEN: {
						Reg16_Node* left_operand = calculated_left_operand;

						switch(calculated_right_operand->token) {
							case NUMBER_TOKEN:
							case CALCULATED_NUMBER_TOKEN: {
								Number_Node* right_operand = calculated_right_operand;

								if(left_operand->index == 0) {
									if(right_operand->value < 256) {
										out(3, 0x83, (3 << 6) | (operation_index << 3) | left_operand->index, right_operand->value);
									}
									else {
										out(3, 0x05 + operation_index * 8, right_operand->value, right_operand->value >> 8);
									}
								}
								else {
									if(right_operand->value < 256) {
										out(3, 0x83, (3 << 6) | (operation_index << 3) | left_operand->index, right_operand->value);
									}
									else {
										out(4, 0x81, (3 << 6) | (operation_index << 3) | left_operand->index, right_operand->value, right_operand->value >> 8);
									}
								}

								break;
							}

							case REG16_TOKEN: {
								Reg16_Node* right_operand = calculated_right_operand;

								out(2, 0x01 + operation_index * 8, (3 << 6) | (right_operand->index << 3) | left_operand->index);

								break;
							}

							case MEM_TOKEN: {
								generate_segment_prefix(calculated_right_operand);
								out(1, 0x3 + operation_index * 8);
								generate_mem(calculated_right_operand, left_operand->index);

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					case MEM_TOKEN: {
						Mem_Node* left_operand = calculated_left_operand;

						switch(calculated_right_operand->token) {
							case NUMBER_TOKEN:
							case CALCULATED_NUMBER_TOKEN: {
								Number_Node* right_operand = calculated_right_operand;

								switch(left_operand->size) {
									case 8: {
										generate_segment_prefix(left_operand);
										out(1, 0x80);
										generate_mem(left_operand, operation_index);
										out(1, right_operand->value);
										
										break;
									}

									case 16: {
										if(right_operand->value < 256) {
											generate_segment_prefix(left_operand);
											out(1, 0x83);
											generate_mem(left_operand, operation_index);
											out(1, right_operand->value);
										}
										else {
											generate_segment_prefix(left_operand);
											out(1, 0x81);
											generate_mem(left_operand, operation_index);
											out(2, right_operand->value, right_operand->value >> 8);
										}

										break;
									}

									default: {
										error(n->left_operand->line_number, "undefined left operand size");
									}
								}

								break;
							}

							case REG16_TOKEN: {
								Reg16_Node* right_operand = calculated_right_operand;

								generate_segment_prefix(left_operand);
								out(1, 0x1 + operation_index * 8);
								generate_mem(left_operand, right_operand->index);

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					default: {
						error(n->left_operand->line_number, "not supported left operand");
					}
				}

				if(calculated_left_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_left_operand);
				}

				if(calculated_right_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_right_operand);
				}

				break;
			}

			case INC_TOKEN: {
				Unary_Operation_Node* n = node;
				Node* calculated_operand = calculate_expression(n->operand);

				switch(calculated_operand->token) {
					case REG8_TOKEN: {
						Reg8_Node* operand = calculated_operand;
						out(2, 0xFE, 0xC0 + operand->index);
						break;
					}

					case REG16_TOKEN: {
						Reg16_Node* operand = calculated_operand;
						out(1, 0x40 + operand->index);
						break;
					}

					case MEM_TOKEN: {
						Mem_Node* operand = calculated_operand;

						switch(operand->size) {
							case 8: {
								generate_segment_prefix(operand);
								out(1, 0xFE);
								generate_mem(operand, 0);
								break;
							}

							case 16: {
								generate_segment_prefix(operand);
								out(1, 0xFF);
								generate_mem(operand, 0);
								break;
							}

							default: {
								error(n->operand->line_number, "inc with not supported operand size");
							}
						}

						break;
					}

					default: {
						error(n->operand->line_number, "inc with not supported operand");
					}
				}

				if(calculated_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_operand);
				}

				break;
			}

			case DEC_TOKEN: {
				Unary_Operation_Node* n = node;
				Node* calculated_operand = calculate_expression(n->operand);

				switch(calculated_operand->token) {
					case REG8_TOKEN: {
						Reg8_Node* operand = calculated_operand;
						out(2, 0xFE, 0xC8 + operand->index);
						break;
					}

					case REG16_TOKEN: {
						Reg16_Node* operand = calculated_operand;
						out(1, 0x48 + operand->index);
						break;
					}

					case MEM_TOKEN: {
						Mem_Node* operand = calculated_operand;

						switch(operand->size) {
							case 8: {
								generate_segment_prefix(operand);
								out(1, 0xFE);
								generate_mem(operand, 1);
								break;
							}

							case 16: {
								generate_segment_prefix(operand);
								out(1, 0xFF);
								generate_mem(operand, 1);
								break;
							}

							default: {
								error(n->operand->line_number, "dec with not supported operand size");
							}
						}

						break;
					}

					default: {
						error(n->operand->line_number, "dec with not supported operand");
					}
				}

				if(calculated_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_operand);
				}

				break;
			}

			case CALL_TOKEN: {
				Unary_Operation_Node* n = node;
				Node* calculated_operand = calculate_expression(n->operand);

				switch(calculated_operand->token) {
					case LONG_ADDRESS_TOKEN: {
						Long_Address_Node* n = calculated_operand;
						Node* calculated_offset = calculate_expression(n->offset);

						switch(calculated_offset->token) {
							case NUMBER_TOKEN:
							case CALCULATED_NUMBER_TOKEN: {
								Number_Node* offset = calculated_offset;

								out(5, 0x9A, offset->value, offset->value >> 8, n->segment, n->segment >> 8);

								break;
							}

							default: {
								error(n->offset->line_number, "call with not supported offset");
							}
						}

						if(calculated_offset->token == CALCULATED_NUMBER_TOKEN) {
							free_memory(calculated_offset);
						}

						break;
					}

					case NUMBER_TOKEN:
					case CALCULATED_NUMBER_TOKEN: {
						Number_Node* operand = calculated_operand;
						Signed_Number offset;

						offset = operand->value - (current_address + 3);

						out(3, 0xE8, offset, offset >> 8);

						break;
					}

					case REG16_TOKEN: {
						Reg16_Node* operand = calculated_operand;
						out(2, 0xFF, (3 << 6) | (2 << 3) | operand->index);
						break;
					}

					case MEM_TOKEN: {
						Mem_Node* operand = calculated_operand;

						generate_segment_prefix(operand);
						out(1, 0xFF);
						generate_mem(calculated_operand, 2);

						break;
					}

					default: {
						error(n->operand->line_number, "call with not supported operand");
					}
				}

				if(calculated_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_operand);
				}

				break;
			}

			case CALLF_TOKEN: {
				Unary_Operation_Node* n = node;
				Node* calculated_operand = calculate_expression(n->operand);

				switch(calculated_operand->token) {
					case MEM_TOKEN: {
						Mem_Node* operand = calculated_operand;

						generate_segment_prefix(operand);
						out(1, 0xFF);
						generate_mem(calculated_operand, 3);

						break;
					}

					default: {
						error(n->operand->line_number, "callf with not supported operand");
					}
				}

				if(calculated_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_operand);
				}

				break;
			}

			case JMP_TOKEN: {
				Unary_Operation_Node* n = node;
				Node* calculated_operand = calculate_expression(n->operand);

				switch(calculated_operand->token) {
					case LONG_ADDRESS_TOKEN: {
						Long_Address_Node* n = calculated_operand;
						Node* calculated_offset = calculate_expression(n->offset);

						switch(calculated_offset->token) {
							case NUMBER_TOKEN:
							case CALCULATED_NUMBER_TOKEN: {
								Number_Node* offset = calculated_offset;

								out(5, 0xEA, offset->value, offset->value >> 8, n->segment, n->segment >> 8);

								break;
							}

							default: {
								error(n->offset->line_number, "jmp with not supported offset");
							}
						}

						if(calculated_offset->token == CALCULATED_NUMBER_TOKEN) {
							free_memory(calculated_offset);
						}

						break;
					}

					case NUMBER_TOKEN:
					case CALCULATED_NUMBER_TOKEN: {
						Number_Node* operand = calculated_operand;
						Signed_Number offset;

						offset = operand->value - (current_address + 2);

						if(offset > 127 || offset < -128) {
							offset = operand->value - (current_address + 3);
							out(3, 0xE9, offset, offset >> 8);
						}
						else {
							out(2, 0xEB, offset);
						}

						break;
					}

					case REG16_TOKEN: {
						Reg16_Node* operand = calculated_operand;
						out(2, 0xFF, (3 << 6) | (4 << 3) | operand->index);
						break;
					}

					case MEM_TOKEN: {
						Mem_Node* operand = calculated_operand;

						generate_segment_prefix(operand);
						out(1, 0xFF);
						generate_mem(calculated_operand, 4);

						break;
					}

					default: {
						error(n->operand->line_number, "call with not supported operand");
					}
				}

				if(calculated_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_operand);
				}

				break;
			}

			case JMPF_TOKEN: {
				Unary_Operation_Node* n = node;
				Node* calculated_operand = calculate_expression(n->operand);

				switch(calculated_operand->token) {
					case MEM_TOKEN: {
						Mem_Node* operand = calculated_operand;

						generate_segment_prefix(operand);
						out(1, 0xFF);
						generate_mem(calculated_operand, 5);

						break;
					}

					default: {
						error(n->operand->line_number, "jmpf with not supported operand");
					}
				}

				if(calculated_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_operand);
				}

				break;
			}

			case PUSH_TOKEN: {
				Unary_Operation_Node* n = node;
				Node* calculated_operand = calculate_expression(n->operand);

				switch(calculated_operand->token) {
					case NUMBER_TOKEN:
					case CALCULATED_NUMBER_TOKEN: {
						Number_Node* operand = calculated_operand;

						if(operand->value < 256) {
							out(2, 0x6A, operand->value);
						}
						else {
							out(3, 0x68, operand->value, operand->value >> 8);
						}

						break;
					}

					case REG16_TOKEN: {
						Reg16_Node* operand = calculated_operand;
						out(1, 0x50 + operand->index);
						break;
					}

					case SEG_TOKEN: {
						Seg_Node* operand = calculated_operand;

						if(operand->index < 4) {
							out(1, 0x06 + operand->index * 8);
						}
						else {
							out(2, 0x0F, 0xA0 + (operand->index - 4) * 8);
						}

						break;
					}

					case MEM_TOKEN: {
						Mem_Node* operand = calculated_operand;

						generate_segment_prefix(operand);
						out(1, 0xFF);
						generate_mem(operand, 6);
						break;
					}

					default: {
						error(n->operand->line_number, "push with not supported operand");
					}
				}

				if(calculated_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_operand);
				}

				break;
			}

			case POP_TOKEN: {
				Unary_Operation_Node* n = node;
				Node* calculated_operand = calculate_expression(n->operand);

				switch(calculated_operand->token) {
					case REG16_TOKEN: {
						Reg16_Node* operand = calculated_operand;
						out(1, 0x58 + operand->index);
						break;
					}

					case SEG_TOKEN: {
						Seg_Node* operand = calculated_operand;

						if(operand->index < 4) {
							out(1, 0x07 + operand->index * 8);
						}
						else {
							out(2, 0x0F, 0xA1 + (operand->index - 4) * 8);
						}

						break;
					}

					case MEM_TOKEN: {
						Mem_Node* operand = calculated_operand;

						generate_segment_prefix(operand);
						out(1, 0x8F);
						generate_mem(operand, 0);
						break;
					}

					default: {
						error(n->operand->line_number, "pop with not supported operand");
					}
				}

				if(calculated_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_operand);
				}

				break;
			}

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
			case JG_TOKEN: {
				Unary_Operation_Node* n = node;
				Node* calculated_operand = calculate_expression(n->operand);
				Number operation_index = node->token - JO_TOKEN;

				switch(calculated_operand->token) {
					case NUMBER_TOKEN:
					case CALCULATED_NUMBER_TOKEN: {
						Number_Node* operand = calculated_operand;

						Signed_Number offset;

						offset = operand->value - (current_address + 2);

						/*if(offset > 127 || offset < -128) {
							offset = operand->value - (current_address + 4);
							out(4, 0x0F, 0x80 + operation_index, offset, offset >> 8);
						}
						else {
							out(2, 0x70 + operation_index, offset);
						}*/

						out(2, 0x70 + operation_index, offset);

						break;
					}

					default: {
						error(n->operand->line_number, "not supported operand");
					}
				}

				if(calculated_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_operand);
				}

				break;
			}

			case TEST_TOKEN: {
				Binary_Operation_Node* n = node;
				Node* calculated_left_operand = calculate_expression(n->left_operand);
				Node* calculated_right_operand = calculate_expression(n->right_operand);

				switch(calculated_left_operand->token) {
					case REG8_TOKEN: {
						Reg8_Node* left_operand = calculated_left_operand;

						switch(calculated_right_operand->token) {
							case NUMBER_TOKEN:
							case CALCULATED_NUMBER_TOKEN: {
								Number_Node* right_operand = calculated_right_operand;

								if(left_operand->index == 0) {
									out(2, 0xA8, right_operand->value);
								}
								else {
									out(3, 0xF6, (3 << 6) | (0 << 3) | left_operand->index, right_operand->value);
								}

								break;
							}

							case REG8_TOKEN: {
								Reg8_Node* right_operand = calculated_right_operand;

								out(2, 0x84, (3 << 6) | (0 << 3) | (right_operand->index << 3) | left_operand->index);

								break;
							}

							case MEM_TOKEN: {
								Mem_Node* right_operand = calculated_right_operand;

								generate_segment_prefix(right_operand);
								out(1, 0x84);
								generate_mem(right_operand, left_operand->index);

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					case REG16_TOKEN: {
						Reg16_Node* left_operand = calculated_left_operand;

						switch(calculated_right_operand->token) {
							case NUMBER_TOKEN:
							case CALCULATED_NUMBER_TOKEN: {
								Number_Node* right_operand = calculated_right_operand;

								if(left_operand->index == 0) {
									out(3, 0xA9, right_operand->value, right_operand->value >> 8);
								}
								else {
									out(4, 0xF7, (3 << 6) | (0 << 3) | left_operand->index, right_operand->value, right_operand->value >> 8);
								}

								break;
							}

							case REG16_TOKEN: {
								Reg16_Node* right_operand = calculated_right_operand;

								out(2, 0x85, (3 << 6) | (right_operand->index << 3) | left_operand->index);

								break;
							}

							case MEM_TOKEN: {
								Mem_Node* right_operand = calculated_right_operand;

								generate_segment_prefix(right_operand);
								out(1, 0x85);
								generate_mem(right_operand, left_operand->index);

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					case MEM_TOKEN: {
						Mem_Node* left_operand = calculated_left_operand;

						switch(calculated_right_operand->token) {
							case NUMBER_TOKEN:
							case CALCULATED_NUMBER_TOKEN: {
								Number_Node* right_operand = calculated_right_operand;

								switch(left_operand->size) {
									case 8: {
										generate_segment_prefix(left_operand);
										out(1, 0xF6);
										generate_mem(left_operand, 0);
										out(1, right_operand->value);
										
										break;
									}

									case 16: {
										generate_segment_prefix(left_operand);
										out(1, 0xF7);
										generate_mem(left_operand, 0);
										out(2, right_operand->value, right_operand->value >> 8);

										break;
									}

									default: {
										error(left_operand->line_number, "undefined left operand size");
									}
								}

								break;
							}

							case REG16_TOKEN: {
								Reg16_Node* right_operand = calculated_right_operand;

								generate_segment_prefix(left_operand);
								out(1, 0x85);
								generate_mem(left_operand, right_operand->index);

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					default: {
						error(n->left_operand->line_number, "not supported left operand");
					}
				}

				if(calculated_left_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_left_operand);
				}

				if(calculated_right_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_right_operand);
				}

				break;
			}

			case XCHG_TOKEN: {
				Binary_Operation_Node* n = node;
				Node* calculated_left_operand = calculate_expression(n->left_operand);
				Node* calculated_right_operand = calculate_expression(n->right_operand);

				switch(calculated_left_operand->token) {
					case REG8_TOKEN: {
						Reg8_Node* left_operand = calculated_left_operand;

						switch(calculated_right_operand->token) {
							case REG8_TOKEN: {
								Reg8_Node* right_operand = calculated_right_operand;

								out(2, 0x86, (3 << 6) | (left_operand->index << 3) | right_operand->index);

								break;
							}

							case MEM_TOKEN: {
								Mem_Node* right_operand = calculated_right_operand;

								generate_segment_prefix(right_operand);
								out(1, 0x86);
								generate_mem(right_operand, left_operand->index);

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					case REG16_TOKEN: {
						Reg16_Node* left_operand = calculated_left_operand;

						switch(calculated_right_operand->token) {
							case REG16_TOKEN: {
								Reg16_Node* right_operand = calculated_right_operand;

								if(left_operand->index == 0) {
									out(1, 0x90 + right_operand->index);
								}
								else if(right_operand->index == 0) {
									out(1, 0x90 + left_operand->index);
								}
								else {
									out(2, 0x87, (3 << 6) | (left_operand->index << 3) | right_operand->index);
								}

								break;
							}

							case MEM_TOKEN: {
								Mem_Node* right_operand = calculated_right_operand;

								generate_segment_prefix(right_operand);
								out(1, 0x87);
								generate_mem(right_operand, left_operand->index);

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					case MEM_TOKEN: {
						Mem_Node* left_operand = calculated_left_operand;

						switch(calculated_right_operand->token) {
							case REG16_TOKEN: {
								Reg16_Node* right_operand = calculated_right_operand;

								generate_segment_prefix(left_operand);
								out(1, 0x87);
								generate_mem(left_operand, right_operand->index);

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					default: {
						error(n->left_operand->line_number, "not supported left operand");
					}
				}

				/*if(calculated_left_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_left_operand);
				}

				if(calculated_right_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_right_operand);
				}*/

				break;
			}

			case MOV_TOKEN: {
				Binary_Operation_Node* n = node;
				Node* calculated_left_operand = calculate_expression(n->left_operand);
				Node* calculated_right_operand = calculate_expression(n->right_operand);

				switch(calculated_left_operand->token) {
					case SEG_TOKEN: {
						Seg_Node* left_operand = calculated_left_operand;

						switch(calculated_right_operand->token) {
							case REG16_TOKEN: {
								Reg16_Node* right_operand = calculated_right_operand;

								out(2, 0x8E, (3 << 6) | (left_operand->index << 3) | right_operand->index);

								break;
							}

							case MEM_TOKEN: {
								Mem_Node* right_operand = calculated_right_operand;

								generate_segment_prefix(right_operand);
								out(1, 0x8E);
								generate_mem(right_operand, left_operand->index);

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					case REG8_TOKEN: {
						Reg8_Node* left_operand = calculated_left_operand;

						switch(calculated_right_operand->token) {
							case NUMBER_TOKEN:
							case CALCULATED_NUMBER_TOKEN: {
								Number_Node* right_operand = calculated_right_operand;

								out(2, 0xB0 + left_operand->index, right_operand->value);

								break;
							}

							case REG8_TOKEN: {
								Reg8_Node* right_operand = calculated_right_operand;

								out(2, 0x88, (3 << 6) | (right_operand->index << 3) | left_operand->index);

								break;
							}

							case MEM_TOKEN: {
								Mem_Node* right_operand = calculated_right_operand;

								generate_segment_prefix(right_operand);
								out(1, 0x8A);
								generate_mem(right_operand, left_operand->index);

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					case REG16_TOKEN: {
						Reg16_Node* left_operand = calculated_left_operand;

						switch(calculated_right_operand->token) {
							case NUMBER_TOKEN:
							case CALCULATED_NUMBER_TOKEN: {
								Number_Node* right_operand = calculated_right_operand;

								out(3, 0xB8 + left_operand->index, right_operand->value, right_operand->value >> 8);

								break;
							}

							case SEG_TOKEN: {
								Seg_Node* right_operand = calculated_right_operand;

								out(2, 0x8C, (3 << 6) | (right_operand->index << 3) | left_operand->index);

								break;
							}

							case REG16_TOKEN: {
								Reg16_Node* right_operand = calculated_right_operand;

								out(2, 0x89, (3 << 6) | (right_operand->index << 3) | left_operand->index);

								break;
							}

							case MEM_TOKEN: {
								Mem_Node* right_operand = calculated_right_operand;

								generate_segment_prefix(right_operand);
								out(1, 0x8B);
								generate_mem(right_operand, left_operand->index);

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					case MEM_TOKEN: {
						Mem_Node* left_operand = calculated_left_operand;

						switch(calculated_right_operand->token) {
							case NUMBER_TOKEN:
							case CALCULATED_NUMBER_TOKEN: {
								Number_Node* right_operand = calculated_right_operand;

								switch(left_operand->size) {
									case 8: {
										generate_segment_prefix(left_operand);
										out(1, 0xC6);
										generate_mem(left_operand, 0);
										out(1, right_operand->value);
										
										break;
									}

									case 16: {
										generate_segment_prefix(left_operand);
										out(1, 0xC7);
										generate_mem(left_operand, 0);
										out(2, right_operand->value, right_operand->value >> 8);

										break;
									}

									default: {
										error(n->left_operand->line_number, "undefined left operand size");
									}
								}

								break;
							}

							case SEG_TOKEN: {
								Seg_Node* right_operand = calculated_right_operand;

								generate_segment_prefix(left_operand);
								out(1, 0x8C);
								generate_mem(left_operand, right_operand->index);

								break;
							}

							case REG16_TOKEN: {
								Reg16_Node* right_operand = calculated_right_operand;

								generate_segment_prefix(left_operand);
								out(1, 0x89);
								generate_mem(left_operand, right_operand->index);

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					default: {
						error(n->left_operand->line_number, "not supported left operand");
					}
				}

				if(calculated_left_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_left_operand);
				}

				if(calculated_right_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_right_operand);
				}

				break;
			}

			case LEA_TOKEN: {
				Binary_Operation_Node* n = node;
				Node* calculated_left_operand = calculate_expression(n->left_operand);
				Node* calculated_right_operand = calculate_expression(n->right_operand);

				switch(calculated_left_operand->token) {
					case REG16_TOKEN: {
						Reg16_Node* left_operand = calculated_left_operand;

						switch(calculated_right_operand->token) {
							case MEM_TOKEN: {
								Mem_Node* right_operand = calculated_right_operand;

								generate_segment_prefix(right_operand);
								out(1, 0x8D);
								generate_mem(right_operand, left_operand->index);

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					default: {
						error(n->left_operand->line_number, "not supported left operand");
					}
				}

				/*if(calculated_left_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_left_operand);
				}

				if(calculated_right_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_right_operand);
				}*/

				break;
			}

			case NOT_TOKEN:
			case NEG_TOKEN:
			case MUL_TOKEN:
			case IMUL_TOKEN:
			case DIV_TOKEN:
			case IDIV_TOKEN: {
				Unary_Operation_Node* n = node;
				Node* calculated_operand = calculate_expression(n->operand);
				Number operation_index = node->token - NOT_TOKEN + 2;

				switch(calculated_operand->token) {
					case REG8_TOKEN: {
						Reg8_Node* operand = calculated_operand;
						out(2, 0xF6, 0xC0 + (operation_index << 3) | operand->index);
						break;
					}

					case REG16_TOKEN: {
						Reg16_Node* operand = calculated_operand;
						out(2, 0xF7, 0xC0 + (operation_index << 3) | operand->index);
						break;
					}

					case MEM_TOKEN: {
						Mem_Node* operand = calculated_operand;

						switch(operand->size) {
							case 8: {
								generate_segment_prefix(operand);
								out(1, 0xF6);
								generate_mem(operand, operation_index);

								break;
							}

							case 16: {
								generate_segment_prefix(operand);
								out(1, 0xF7);
								generate_mem(operand, operation_index);

								break;
							}

							default: {
								error(n->operand->line_number, "undefined operand size");
							}
						}

						break;
					}

					default: {
						error(n->operand->line_number, "not supported operand");
					}
				}

				/*if(calculated_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_operand);
				}*/

				break;
			}

			case ROL_TOKEN:
			case ROR_TOKEN:
			case RCL_TOKEN:
			case RCR_TOKEN:
			case SHL_TOKEN:
			case SHR_TOKEN:
			case SAL_TOKEN:
			case SAR_TOKEN: {
				Binary_Operation_Node* n = node;
				Node* calculated_left_operand = calculate_expression(n->left_operand);
				Node* calculated_right_operand = calculate_expression(n->right_operand);
				Number operation_index = node->token - ROL_TOKEN;

				switch(calculated_left_operand->token) {
					case REG8_TOKEN: {
						Reg8_Node* left_operand = calculated_left_operand;

						switch(calculated_right_operand->token) {
							case NUMBER_TOKEN:
							case CALCULATED_NUMBER_TOKEN: {
								Number_Node* right_operand = calculated_right_operand;

								if(right_operand->value == 1) {
									out(2, 0xD0, (3 << 6) | (operation_index << 3) | left_operand->index);
								}
								else {
									out(3, 0xC0, (3 << 6) | (operation_index << 3) | left_operand->index, right_operand->value);
								}

								break;
							}

							case REG8_TOKEN: {
								Reg8_Node* right_operand = calculated_right_operand;

								if(right_operand->index != 1) {
									error(n->right_operand->line_number, "only CL supported for rotations");
								}

								out(2, 0xD2, (3 << 6) | (operation_index << 3) | left_operand->index);

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					case REG16_TOKEN: {
						Reg16_Node* left_operand = calculated_left_operand;

						switch(calculated_right_operand->token) {
							case NUMBER_TOKEN:
							case CALCULATED_NUMBER_TOKEN: {
								Number_Node* right_operand = calculated_right_operand;

								if(right_operand->value == 1) {
									out(2, 0xD1, (3 << 6) | (operation_index << 3) | left_operand->index);
								}
								else {
									out(3, 0xC1, (3 << 6) | (operation_index << 3) | left_operand->index, right_operand->value);
								}

								break;
							}

							case REG8_TOKEN: {
								Reg8_Node* right_operand = calculated_right_operand;

								if(right_operand->index != 1) {
									error(n->right_operand->line_number, "only CL supported for rotations");
								}

								out(2, 0xD3, (3 << 6) | (operation_index << 3) | left_operand->index);

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					case MEM_TOKEN: {
						Mem_Node* left_operand = calculated_left_operand;

						switch(calculated_right_operand->token) {
							case NUMBER_TOKEN:
							case CALCULATED_NUMBER_TOKEN: {
								Number_Node* right_operand = calculated_right_operand;

								switch(left_operand->size) {
									case 8: {
										if(right_operand->value == 1) {
											generate_segment_prefix(left_operand);
											out(1, 0xD0);
											generate_mem(left_operand, operation_index);
										}
										else {
											generate_segment_prefix(left_operand);
											out(1, 0xC0);
											generate_mem(left_operand, operation_index);
											out(1, right_operand->value);
										}
										
										break;
									}

									case 16: {
										if(right_operand->value == 1) {
											generate_segment_prefix(left_operand);
											out(1, 0xD1);
											generate_mem(left_operand, operation_index);
										}
										else {
											generate_segment_prefix(left_operand);
											out(1, 0xC1);
											generate_mem(left_operand, operation_index);
											out(1, right_operand->value);
										}

										break;
									}

									default: {
										error(n->left_operand->line_number, "undefined left operand size");
									}
								}

								break;
							}

							case REG8_TOKEN: {
								Reg8_Node* right_operand = calculated_right_operand;

								if(right_operand->index != 1) {
									error(n->right_operand->line_number, "only CL supported for rotations");
								}

								switch(left_operand->size) {
									case 8: {
										generate_segment_prefix(left_operand);
										out(1, 0xD2);
										generate_mem(left_operand, operation_index);
										
										break;
									}

									case 16: {
										generate_segment_prefix(left_operand);
										out(1, 0xD3);
										generate_mem(left_operand, operation_index);

										break;
									}

									default: {
										error(n->left_operand->line_number, "undefined left operand size");
									}
								}

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					default: {
						error(n->left_operand->line_number, "not supported left operand");
					}
				}

				if(calculated_left_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_left_operand);
				}

				if(calculated_right_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_right_operand);
				}

				break;
			}

			case DAA_TOKEN: {
				out(1, 0x27);
				break;
			}

			case DAS_TOKEN: {
				out(1, 0x2F);
				break;
			}

			case AAA_TOKEN: {
				out(1, 0x37);
				break;
			}

			case AAS_TOKEN: {
				out(1, 0x3F);
				break;
			}

			case CBW_TOKEN: {
				out(1, 0x98);
				break;
			}

			case CWD_TOKEN: {
				out(1, 0x99);
				break;
			}

			case WAIT_TOKEN: {
				out(1, 0x9B);
				break;
			}

			case PUSHF_TOKEN: {
				out(1, 0x9C);
				break;
			}

			case POPF_TOKEN: {
				out(1, 0x9D);
				break;
			}

			case SAHF_TOKEN: {
				out(1, 0x9E);
				break;
			}

			case LAHF_TOKEN: {
				out(1, 0x9F);
				break;
			}

			case MOVSB_TOKEN: {
				out(1, 0xA4);
				break;
			}

			case MOVSW_TOKEN: {
				out(1, 0xA5);
				break;
			}

			case CMPSB_TOKEN: {
				out(1, 0xA6);
				break;
			}

			case CMPSW_TOKEN: {
				out(1, 0xA7);
				break;
			}

			case STOSB_TOKEN: {
				out(1, 0xAA);
				break;
			}

			case STOSW_TOKEN: {
				out(1, 0xAB);
				break;
			}

			case LODSB_TOKEN: {
				out(1, 0xAC);
				break;
			}

			case LODSW_TOKEN: {
				out(1, 0xAD);
				break;
			}

			case SCASB_TOKEN: {
				out(1, 0xAE);
				break;
			}

			case SCASW_TOKEN: {
				out(1, 0xAF);
				break;
			}

			case RETN_TOKEN: {
				Unary_Operation_Node* n = node;
				Node* calculated_operand = calculate_expression(n->operand);

				switch(calculated_operand->token) {
					case NUMBER_TOKEN:
					case CALCULATED_NUMBER_TOKEN: {
						Number_Node* operand = calculated_operand;

						out(3, 0xC2, operand->value, operand->value >> 8);

						break;
					}

					default: {
						error(n->operand->line_number, "retn with not supported operand");
					}
				}

				if(calculated_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_operand);
				}

				break;
			}

			case RET_TOKEN: {
				out(1, 0xC3);
				break;
			}

			case RETFN_TOKEN: {
				Unary_Operation_Node* n = node;
				Node* calculated_operand = calculate_expression(n->operand);

				switch(calculated_operand->token) {
					case NUMBER_TOKEN:
					case CALCULATED_NUMBER_TOKEN: {
						Number_Node* operand = calculated_operand;

						out(3, 0xCA, operand->value, operand->value >> 8);

						break;
					}

					default: {
						error(n->operand->line_number, "retfn with not supported operand");
					}
				}

				if(calculated_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_operand);
				}

				break;
			}

			case RETF_TOKEN: {
				out(1, 0xCB);
				break;
			}

			case INT3_TOKEN: {
				out(1, 0xCC);
				break;
			}

			case INT_TOKEN: {
				Unary_Operation_Node* n = node;
				Node* calculated_operand = calculate_expression(n->operand);

				switch(calculated_operand->token) {
					case NUMBER_TOKEN:
					case CALCULATED_NUMBER_TOKEN: {
						Number_Node* operand = calculated_operand;

						out(2, 0xCD, operand->value);

						break;
					}

					default: {
						error(n->operand->line_number, "int with not supported operand");
					}
				}

				if(calculated_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_operand);
				}

				break;
			}

			case INTO_TOKEN: {
				out(1, 0xCE);
				break;
			}

			case IRET_TOKEN: {
				out(1, 0xCF);
				break;
			}

			case LOOPNZ_TOKEN:
			case LOOPZ_TOKEN:
			case LOOP_TOKEN:
			case JCXZ_TOKEN: {
				Unary_Operation_Node* n = node;
				Node* calculated_operand = calculate_expression(n->operand);
				Number operation_index = node->token - LOOPNZ_TOKEN;

				switch(calculated_operand->token) {
					case NUMBER_TOKEN:
					case CALCULATED_NUMBER_TOKEN: {
						Number_Node* operand = calculated_operand;

						Signed_Number offset;

						offset = operand->value - (current_address + 2);

						out(2, 0xE0 + operation_index, offset);

						break;
					}

					default: {
						error(n->operand->line_number, "not supported operand");
					}
				}

				if(calculated_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_operand);
				}

				break;
			}

			case IN_TOKEN: {
				Binary_Operation_Node* n = node;
				Node* calculated_left_operand = calculate_expression(n->left_operand);
				Node* calculated_right_operand = calculate_expression(n->right_operand);

				switch(calculated_left_operand->token) {
					case REG8_TOKEN: {
						Reg8_Node* left_operand = calculated_left_operand;

						if(left_operand->index != 0) {
							error(n->left_operand->line_number, "not supported left operand, use AL or AX");
						}

						switch(calculated_right_operand->token) {
							case NUMBER_TOKEN:
							case CALCULATED_NUMBER_TOKEN: {
								Number_Node* right_operand = calculated_right_operand;

								out(2, 0xE4, right_operand->value);

								break;
							}

							case REG8_TOKEN: {
								Reg8_Node* right_operand = calculated_right_operand;

								if(right_operand->index != 2) {
									error(n->right_operand->line_number, "not supported right operand, use DX or imm8");
								}

								out(1, 0xEC);

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					case REG16_TOKEN: {
						Reg16_Node* left_operand = calculated_left_operand;

						if(left_operand->index != 0) {
							error(n->left_operand->line_number, "not supported left operand, use AL or AX");
						}

						switch(calculated_right_operand->token) {
							case NUMBER_TOKEN:
							case CALCULATED_NUMBER_TOKEN: {
								Number_Node* right_operand = calculated_right_operand;

								out(2, 0xE5, right_operand->value);

								break;
							}

							case REG16_TOKEN: {
								Reg16_Node* right_operand = calculated_right_operand;

								if(right_operand->index != 2) {
									error(n->right_operand->line_number, "not supported right operand, use DX or imm8");
								}

								out(1, 0xED);

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					default: {
						error(n->left_operand->line_number, "not supported left operand");
					}
				}

				if(calculated_left_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_left_operand);
				}

				if(calculated_right_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_right_operand);
				}

				break;
			}

			case OUT_TOKEN: {
				Binary_Operation_Node* n = node;
				Node* calculated_left_operand = calculate_expression(n->left_operand);
				Node* calculated_right_operand = calculate_expression(n->right_operand);

				switch(calculated_left_operand->token) {
					case NUMBER_TOKEN:
					case CALCULATED_NUMBER_TOKEN: {
						Number_Node* left_operand = calculated_left_operand;

						switch(calculated_right_operand->token) {
							case REG8_TOKEN: {
								Reg8_Node* right_operand = calculated_right_operand;

								if(right_operand->index != 0) {
									error(n->right_operand->line_number, "not supported right operand, use AL or AX");
								}

								out(2, 0xE6, left_operand->value);

								break;
							}

							case REG16_TOKEN: {
								Reg16_Node* right_operand = calculated_right_operand;

								if(right_operand->index != 0) {
									error(n->right_operand->line_number, "not supported right operand, use AL or AX");
								}

								out(2, 0xE7, left_operand->value);

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					case REG16_TOKEN: {
						Reg16_Node* left_operand = calculated_left_operand;

						if(left_operand->index != 2) {
							error(n->left_operand->line_number, "not supported left operand, use DX or imm8");
						}

						switch(calculated_right_operand->token) {
							case REG8_TOKEN: {
								Reg8_Node* right_operand = calculated_right_operand;

								if(right_operand->index != 0) {
									error(n->right_operand->line_number, "not supported right operand, use AL or AX");
								}

								out(1, 0xEE);

								break;
							}

							case REG16_TOKEN: {
								Reg16_Node* right_operand = calculated_right_operand;

								if(right_operand->index != 0) {
									error(n->right_operand->line_number, "not supported right operand, use AL or AX");
								}

								out(1, 0xEF);

								break;
							}

							default: {
								error(n->right_operand->line_number, "not supported right operand");
							}
						}

						break;
					}

					default: {
						error(n->left_operand->line_number, "not supported left operand");
					}
				}

				if(calculated_left_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_left_operand);
				}

				if(calculated_right_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_right_operand);
				}

				break;
			}

			case LOCK_TOKEN: {
				out(1, 0xF0);
				break;
			}

			case INT1_TOKEN: {
				out(1, 0xF1);
				break;
			}

			case REPNE_TOKEN: {
				out(1, 0xF2);
				break;
			}

			case REP_TOKEN: {
				out(1, 0xF3);
				break;
			}

			case HLT_TOKEN: {
				out(1, 0xF4);
				break;
			}

			case CMC_TOKEN: {
				out(1, 0xF5);
				break;
			}

			case CLC_TOKEN: {
				out(1, 0xF8);
				break;
			}

			case STC_TOKEN: {
				out(1, 0xF9);
				break;
			}

			case CLI_TOKEN: {
				out(1, 0xFA);
				break;
			}

			case STI_TOKEN: {
				out(1, 0xFB);
				break;
			}

			case CLD_TOKEN: {
				out(1, 0xFC);
				break;
			}

			case STD_TOKEN: {
				out(1, 0xFD);
				break;
			}

			case ORG_TOKEN: {
				Unary_Operation_Node* n = node;
				Node* calculated_operand = calculate_expression(n->operand);
				Number operation_index = node->token - LOOPNZ_TOKEN;

				switch(calculated_operand->token) {
					case NUMBER_TOKEN:
					case CALCULATED_NUMBER_TOKEN: {
						Number_Node* operand = calculated_operand;

						current_address = operand->value;

						break;
					}

					default: {
						error(n->operand->line_number, "not supported operand");
					}
				}

				if(calculated_operand->token == CALCULATED_NUMBER_TOKEN) {
					free_memory(calculated_operand);
				}

				break;
			}

			case DB_TOKEN: {
				break;
			}

			default: {
				Node* calculated_data = calculate_expression(node);

				switch(calculated_data->token) {
					case NUMBER_TOKEN:
					case CALCULATED_NUMBER_TOKEN: {
						Number_Node* data = calculated_data;
						out(1, data->value);
						break;
					}

					default: {
						error(node->line_number, "not calculatable expression");
					}
				}
			}
		}
	}
}