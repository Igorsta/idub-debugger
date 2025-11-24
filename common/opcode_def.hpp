#pragma once

#define STCK_INIT	init
#define STCK_DEINIT deinit
#define REG_TO_STCK reg_to_stack
#define STCK_TO_REG stack_to_reg
#define REG_TO_RVAL reg_to_retval
#define APPLY_TO_MEM_STACK(macro)                                                                  \
	macro(STCK_INIT) macro(STCK_DEINIT) macro(REG_TO_STCK) macro(STCK_TO_REG) macro(REG_TO_RVAL)

#define REG_FROM_REG reg_from_reg
#define INIT_IMM	 init_imm
#define INPUT_TO_REG input_reg
#define OUTPUT_REG	 output_reg
#define APPLY_TO_REG(macro)                                                                        \
	macro(REG_FROM_REG) macro(INIT_IMM) macro(INPUT_TO_REG) macro(OUTPUT_REG)

#define CMP_REG					   cmp_reg
#define CALL					   call_func
#define FUNC_RET				   func_return
#define EXIT_PROG				   exit_prog
#define APPLY_TO_CALL_STACK(macro) macro(CMP_REG) macro(CALL) macro(FUNC_RET) macro(EXIT_PROG)

#define ADD_IN_PLACE add_in_place
#define MUL_IN_PLACE mul_in_place
#define SUB_IN_PLACE sub_in_place
#define MOD_IN_PLACE mod_in_place
#define DIV_IN_PLACE div_in_place
#define APPLY_TO_IN_PLACE_ARITH(macro)                                                             \
	macro(ADD_IN_PLACE) macro(MUL_IN_PLACE) macro(SUB_IN_PLACE) macro(MOD_IN_PLACE)                \
		macro(DIV_IN_PLACE)

#define ALLOC_HEAP	 alloc_heap
#define DEALLOC_HEAP dealloc_heap
#define READ_HEAP	 read_heap
#define WRITE_HEAP	 write_heap
#define APPLY_TO_HEAP(macro)                                                                       \
	macro(ALLOC_HEAP) macro(DEALLOC_HEAP) macro(READ_HEAP) macro(WRITE_HEAP)

#define JUMP				 jump_to
#define JUMP_EQ				 jump_if_eq
#define APPLY_TO_JUMP(macro) macro(JUMP) macro(JUMP_EQ)

#define LABEL				  label
#define DEMAND_REG			  demand_reg
#define APPLY_TO_OTHER(macro) macro(LABEL) macro(DEMAND_REG)

#define APPLY_TO_SIMPLE(macro)                                                                     \
	APPLY_TO_CALL_STACK(macro)                                                                     \
	APPLY_TO_HEAP(macro)                                                                           \
	APPLY_TO_IN_PLACE_ARITH(macro)                                                                 \
	APPLY_TO_REG(macro)                                                                            \
	APPLY_TO_MEM_STACK(macro)

#define APPLY_TO_EXEC(macro)                                                                       \
	APPLY_TO_SIMPLE(macro)                                                                         \
	APPLY_TO_JUMP(macro)

#define APPLY_TO_ALL(macro)                                                                        \
	APPLY_TO_EXEC(macro)                                                                         \
	APPLY_TO_OTHER(macro)
