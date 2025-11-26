#include "opcode.hpp"
#include "boilerplate.hpp"

//////////////// PARSE ///////////////////

APPLY_TO_SIMPLE(PARSE_SIMPLE_IMPL)
APPLY_TO_JUMP(PARSE_JUMPS_IMPL)


//////////////// OTHER ///////////////////

ENLIST_OPERANDS(LABEL, operand_t::LABEL_ID)
void _N_PARSE::LABEL(PARSE_OPCODE_ARGS) { builder.func_builder.define_labl(matches[1]); }
ENLIST_OPERANDS(DEMAND_REG, operand_t::REGISTER_ID)
void _N_PARSE::DEMAND_REG(PARSE_OPCODE_ARGS) { builder.func_builder.define_reg(matches[1]); }


//////////////// PRNT ///////////////////

APPLY_TO_EXEC(PRNT_IMPL)

//////////////// EXEC ///////////////////


ENLIST_OPERANDS(STCK_INIT)
_N_EXEC::ret_t _N_EXEC::STCK_INIT(RAW_EXEC_ARGS) {
	auto end = mem_space->MEM_STACK.get() + mem_space->MEM_STACK_SIZE;
	CORE_ASSERT(frame->stack_head + 1 < end, "Stack overflow");

	frame->stack_head++;

	return 1;
}

ENLIST_OPERANDS(STCK_DEINIT)
_N_EXEC::ret_t _N_EXEC::STCK_DEINIT(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;
	frame->stack_head = last_on_stck(FWD_RAW_ARGS);

	return 1;
}

ENLIST_OPERANDS(REG_TO_STCK, operand_t::REGISTER_ID)
_N_EXEC::ret_t _N_EXEC::REG_TO_STCK(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;
	auto arg0 = instr->arg[0];

	auto &stack_top = *last_on_stck(FWD_RAW_ARGS);
	auto &reg = get_reg(arg0, FWD_RAW_ARGS);

	stack_top = reg;

	return 1;
}

ENLIST_OPERANDS(STCK_TO_REG, operand_t::REGISTER_ID)
_N_EXEC::ret_t _N_EXEC::STCK_TO_REG(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];

	auto &stack_top = *last_on_stck(FWD_RAW_ARGS);
	auto &reg = get_reg(arg0, FWD_RAW_ARGS);

	reg = stack_top;

	return 1;
}

ENLIST_OPERANDS(REG_FROM_REG, operand_t::REGISTER_ID, operand_t::REGISTER_ID)
_N_EXEC::ret_t _N_EXEC::REG_FROM_REG(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];

	auto &reg0 = get_reg(arg0, FWD_RAW_ARGS);
	auto &reg1 = get_reg(arg1, FWD_RAW_ARGS);

	reg0 = reg1;

	return 1;
}

ENLIST_OPERANDS(REG_TO_RVAL, operand_t::REGISTER_ID)
_N_EXEC::ret_t _N_EXEC::REG_TO_RVAL(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];

	auto &stack_bottom = *frame->stack_start;
	auto &reg = get_reg(arg0, FWD_RAW_ARGS);
	;

	stack_bottom = reg;

	return 1;
}

ENLIST_OPERANDS(INPUT_TO_REG, operand_t::REGISTER_ID)
_N_EXEC::ret_t _N_EXEC::INPUT_TO_REG(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];

	auto &reg = get_reg(arg0, FWD_RAW_ARGS);

	size_t val;
	(mem_space->IO) >> val;

	reg = val;

	return 1;
}

ENLIST_OPERANDS(OUTPUT_REG, operand_t::REGISTER_ID)
_N_EXEC::ret_t _N_EXEC::OUTPUT_REG(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];

	auto &reg = get_reg(arg0, FWD_RAW_ARGS);

	std::cout << reg << std::endl;

	return 1;
}

ENLIST_OPERANDS(CALL, operand_t::FUNC_NAME)
_N_EXEC::ret_t _N_EXEC::CALL(RAW_EXEC_ARGS) {
	auto arg0 = instr->arg[0];
	auto it = avail_funcs->find(arg0);
	CORE_ASSERT(it != avail_funcs->end(), "Call to undeclared function");

	frame->instr = instr;

	auto prev_frame = frame;
	auto frame_end = mem_space->CALL_STACK.get() + mem_space->CALL_STACK_SIZE;
	CORE_ASSERT(frame + 1 < frame_end, "Call stack overflow");
	frame++;

	frame->stack_head = prev_frame->stack_head;

	auto &[_, callee] = *it;

	auto shared_memory = callee.no_of_args;
	CORE_ASSERT(prev_frame->stack_start + shared_memory <= prev_frame->stack_head,
				"I guess check number of elements on stack?"); //@todo: WTH is going on here?
	frame->stack_start = frame->stack_head - shared_memory;

	instr = callee.body.data();

	return 0;
}

ENLIST_OPERANDS(FUNC_RET)
_N_EXEC::ret_t _N_EXEC::FUNC_RET(RAW_EXEC_ARGS) {
	auto end = mem_space->MEM_STACK.get() + mem_space->MEM_STACK_SIZE;
	auto &func = (*avail_funcs)[frame->cur_func_id];

	auto expected_head = frame->stack_start + func.res_size;
	CORE_ASSERT(expected_head == frame->stack_head,
				"Function returned wrong number of elements on a stack");

	frame--;
	frame->stack_head = expected_head;
	instr = frame->instr;

	return 1;
}

ENLIST_OPERANDS(EXIT_PROG, operand_t::REGISTER_ID)
_N_EXEC::ret_t _N_EXEC::EXIT_PROG(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];
	auto ret_val = get_reg(arg0, FWD_RAW_ARGS);

	throw ret_val;
}

ENLIST_OPERANDS(JUMP, operand_t::LABEL_ID)
_N_EXEC::ret_t _N_EXEC::JUMP(RAW_EXEC_ARGS) { return instr->arg[0]; }

ENLIST_OPERANDS(CMP_REG, operand_t::REGISTER_ID, operand_t::REGISTER_ID)
_N_EXEC::ret_t _N_EXEC::CMP_REG(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto &arg0 = instr->arg[0];
	auto &arg1 = instr->arg[1];

	const auto &reg0 = get_reg(arg0, FWD_RAW_ARGS);
	const auto &reg1 = get_reg(arg1, FWD_RAW_ARGS);

	frame->flags.were_equal = (reg0 == reg1);
	frame->flags.first_was_bigger = (reg0 > reg1);

	return 1;
}

ENLIST_OPERANDS(JUMP_EQ, operand_t::LABEL_ID)
_N_EXEC::ret_t _N_EXEC::JUMP_EQ(RAW_EXEC_ARGS) {
	return (frame->flags.were_equal) ? instr->arg[0] : 1;
}

ENLIST_OPERANDS(INIT_IMM, operand_t::REGISTER_ID, operand_t::DECIMAL_NUM)
_N_EXEC::ret_t _N_EXEC::INIT_IMM(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];

	auto &reg = get_reg(arg0, FWD_RAW_ARGS);

	reg = arg1;

	return 1;
}

ENLIST_OPERANDS(ALLOC_HEAP, operand_t::REG_WITH_PTR, operand_t::REGISTER_ID)
_N_EXEC::ret_t _N_EXEC::ALLOC_HEAP(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];

	auto &reg0 = get_reg(arg0, FWD_RAW_ARGS);
	auto &reg1 = get_reg(arg1, FWD_RAW_ARGS);

	auto ptr = mem_space->HEAP.allocate(reg1);
	reg0 = ptr;

	return 1;
}

ENLIST_OPERANDS(DEALLOC_HEAP, operand_t::REG_WITH_PTR)
_N_EXEC::ret_t _N_EXEC::DEALLOC_HEAP(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];

	auto reg = get_reg(arg0, FWD_RAW_ARGS);

	mem_space->HEAP.deallocate(reg);

	return 1;
}

ENLIST_OPERANDS(WRITE_HEAP, operand_t::REG_WITH_PTR, operand_t::REGISTER_ID)
_N_EXEC::ret_t _N_EXEC::WRITE_HEAP(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];

	auto &reg0 = get_reg(arg0, FWD_RAW_ARGS);
	auto &reg1 = get_reg(arg1, FWD_RAW_ARGS);

	CORE_ASSERT(mem_space->HEAP.write(reg0, reg1), "Unsuccessful write to heap");

	return 1;
}

ENLIST_OPERANDS(READ_HEAP, operand_t::REGISTER_ID, operand_t::REG_WITH_PTR)
_N_EXEC::ret_t _N_EXEC::READ_HEAP(RAW_EXEC_ARGS) {
	using namespace _N_EXEC_UTILS;

	auto arg0 = instr->arg[0];
	auto arg1 = instr->arg[1];

	auto &reg0 = get_reg(arg0, FWD_RAW_ARGS);
	auto &reg1 = get_reg(arg1, FWD_RAW_ARGS);

	auto opt = mem_space->HEAP.read(reg1);
	CORE_ASSERT(opt.has_value(), "Accessing a forbidden part of heap {}", reg1);
	reg0 = opt.value();

	return 1;
}


ENLIST_OPERANDS(ADD_IN_PLACE, operand_t::REGISTER_ID, operand_t::REGISTER_ID)
ENLIST_OPERANDS(MUL_IN_PLACE, operand_t::REGISTER_ID, operand_t::REGISTER_ID)
ENLIST_OPERANDS(DIV_IN_PLACE, operand_t::REGISTER_ID, operand_t::REGISTER_ID)
ENLIST_OPERANDS(SUB_IN_PLACE, operand_t::REGISTER_ID, operand_t::REGISTER_ID)
ENLIST_OPERANDS(MOD_IN_PLACE, operand_t::REGISTER_ID, operand_t::REGISTER_ID)

EXEC_ARITH_IN_PLACE_IMPL(ADD_IN_PLACE, +=)
EXEC_ARITH_IN_PLACE_IMPL(MUL_IN_PLACE, *=)
EXEC_ARITH_IN_PLACE_IMPL(SUB_IN_PLACE, -=)
EXEC_ARITH_IN_PLACE_IMPL(DIV_IN_PLACE, /=)
EXEC_ARITH_IN_PLACE_IMPL(MOD_IN_PLACE, %=)