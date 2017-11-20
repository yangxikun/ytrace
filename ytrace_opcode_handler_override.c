/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2016 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.0 of the Xdebug license,    |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://xdebug.derickrethans.nl/license.php                           |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | xdebug@derickrethans.nl so we can mail you a copy immediately.       |
   +----------------------------------------------------------------------+
   | Authors: Derick Rethans <derick@xdebug.org>                          |
   +----------------------------------------------------------------------+
 */

#include <stdio.h>
#include "php.h"
#include "php_ytrace.h"
#include "ytrace_compat.h"
#include "zend_execute.h"
#include "ytrace_str.h"
#include "ytrace_filter.h"

ZEND_EXTERN_MODULE_GLOBALS(ytrace)

#if PHP_VERSION_ID >= 70000
static int ytrace_is_static_call(const zend_op *cur_opcode, const zend_op *prev_opcode, const zend_op **found_opcode)
{
	/*
	const zend_op *opcode_ptr;

	opcode_ptr = cur_opcode;
# if PHP_VERSION_ID >= 70100
	while (!(opcode_ptr->opcode == ZEND_EXT_STMT) && !((opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_W) || (opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_RW))) {
		opcode_ptr = opcode_ptr - 1;
	}
	if ((opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_W) || (opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_RW)) {
# else
	while (!(opcode_ptr->opcode == ZEND_EXT_STMT) && !((opcode_ptr->opcode == ZEND_FETCH_W) || (opcode_ptr->opcode == ZEND_FETCH_RW))) {
		opcode_ptr = opcode_ptr - 1;
	}
	if (((opcode_ptr->opcode == ZEND_FETCH_W) || (opcode_ptr->opcode == ZEND_FETCH_RW)) && opcode_ptr->extended_value == ZEND_FETCH_STATIC_MEMBER) {
# endif
		*found_opcode = opcode_ptr;
		return 1;
	}
	*/
# if PHP_VERSION_ID >= 70100
	if ((prev_opcode->opcode == ZEND_FETCH_STATIC_PROP_W) || (prev_opcode->opcode == ZEND_FETCH_STATIC_PROP_RW)) {
# else
	if (((prev_opcode->opcode == ZEND_FETCH_W) || (prev_opcode->opcode == ZEND_FETCH_RW)) && prev_opcode->extended_value == ZEND_FETCH_STATIC_MEMBER) {
# endif
		*found_opcode = prev_opcode;
		return 1;
	}
	return 0;
}
#else
static int ytrace_is_static_call(const zend_op *cur_opcode, const zend_op *prev_opcode, const zend_op **found_opcode)
{
	const zend_op *opcode_ptr;

	opcode_ptr = prev_opcode;
	while (opcode_ptr->opcode == ZEND_FETCH_DIM_W || opcode_ptr->opcode == ZEND_FETCH_OBJ_W || opcode_ptr->opcode == ZEND_FETCH_W || opcode_ptr->opcode == ZEND_FETCH_RW) {
		opcode_ptr = opcode_ptr - 1;
	}
	opcode_ptr++;

	if (opcode_ptr->op1_type == IS_CONST && opcode_ptr->extended_value == ZEND_FETCH_STATIC_MEMBER)
	{
		*found_opcode = cur_opcode;
		return 1;
	}
	return 0;
}
#endif

/* {{{ ytrace_include_or_eval_handler(ZEND_USER_OPCODE_HANDLER_ARGS)
 */
static int ytrace_include_or_eval_handler(ZEND_USER_OPCODE_HANDLER_ARGS)
{
	if (!YTRACE_G(trace_file)) {
		return ZEND_USER_OPCODE_DISPATCH;
	}
	const zend_op *cur_opcode = execute_data->opline;
	zend_op_array *op_array;

	ytrace_str str;

	/* filename */
#if PHP_VERSION_ID < 70000
	op_array = execute_data->op_array;
#else
	if (execute_data->func && ZEND_USER_CODE(execute_data->func->common.type)) {
		op_array = &(execute_data->func->op_array);
	} else if (execute_data->prev_execute_data && execute_data->prev_execute_data->func &&
			ZEND_USER_CODE(execute_data->prev_execute_data->func->common.type)) {
		op_array = &(execute_data->prev_execute_data->func->op_array); /* try using prev */
	}
#endif
	char *filename = STR_NAME_VAL(op_array->filename);

	if (!ytrace_should_trace(filename)) return ZEND_USER_OPCODE_DISPATCH;
	ytrace_str_new(&str);

	ytrace_str_append(&str, filename, 0);
	ytrace_str_append(&str, "\t", 0);
	
	switch (cur_opcode->extended_value) {
		case ZEND_INCLUDE_ONCE:
			ytrace_str_append(&str, ytrace_sprintf("%zu\tI\t%zu\tinclude_once\t", cur_opcode->lineno, YTRACE_G(level) + 1), 1);
			break;
		case ZEND_REQUIRE_ONCE:
			ytrace_str_append(&str, ytrace_sprintf("%zu\tI\t%zu\trequire_once\t", cur_opcode->lineno, YTRACE_G(level) + 1), 1);
			break;
		case ZEND_INCLUDE:
			ytrace_str_append(&str, ytrace_sprintf("%zu\tI\t%zu\tinclude\t", cur_opcode->lineno, YTRACE_G(level) + 1), 1);
			break;
		case ZEND_REQUIRE:
			ytrace_str_append(&str, ytrace_sprintf("%zu\tI\t%zu\trequire\t", cur_opcode->lineno, YTRACE_G(level) + 1), 1);
			break;
		case ZEND_EVAL:
			YTRACE_G(in_eval) = 1;
			ytrace_str_append(&str, ytrace_sprintf("%zu\tE\t%zu\t{eval}\t", cur_opcode->lineno, YTRACE_G(level) + 1), 1);
			break;
	}

	if (cur_opcode->extended_value == ZEND_EVAL) {
		zval *inc_filename;
		zval tmp_inc_filename;

		inc_filename = ytrace_get_zval(execute_data, cur_opcode->op1_type, &cur_opcode->op1);

		/* If there is no inc_filename, we're just bailing out instead */
		if (!inc_filename) {
			return ZEND_USER_OPCODE_DISPATCH;
		}

		if (Z_TYPE_P(inc_filename) != IS_STRING) {
			tmp_inc_filename = *inc_filename;
			zval_copy_ctor(&tmp_inc_filename);
			convert_to_string(&tmp_inc_filename);
			inc_filename = &tmp_inc_filename;
		}

		ytrace_str_append(&str, Z_STRVAL_P(inc_filename), 0);

		if (inc_filename == &tmp_inc_filename) {
			zval_dtor(&tmp_inc_filename);
		}
	} else {
		ytrace_get_zval_value(ytrace_get_zval(execute_data, cur_opcode->op1_type, &cur_opcode->op1), &str);
	}
	
	ytrace_str_append(&str, "\n", 0);
	if (YTRACE_G(trace_file)) {
		fwrite(str.c, 1, str.len, YTRACE_G(trace_file));
	}
	ytrace_str_destroy(&str);
	return ZEND_USER_OPCODE_DISPATCH;
}
/* }}} */

/* {{{ ytrace_find_var_name(ytrace_str *name, zend_execute_data *execute_data TSRMLS_DC)
 */
static void *ytrace_find_var_name(ytrace_str *name, zend_execute_data *execute_data TSRMLS_DC)
{
	const zend_op *cur_opcode, *next_opcode, *prev_opcode = NULL, *opcode_ptr;
	zval          *dimval;
	int            is_var;
#if PHP_VERSION_ID >= 70000
	zend_op_array *op_array = &execute_data->func->op_array;
#else
	int cv_len;
	zend_op_array *op_array = execute_data->op_array;
#endif
	int            gohungfound = 0, is_static = 0;
	char          *zval_value = NULL;
	const zend_op *static_opcode_ptr;

#if PHP_VERSION_ID >= 70000
	cur_opcode = execute_data->opline;
#else
	cur_opcode = *EG(opline_ptr);
#endif
	next_opcode = cur_opcode + 1;
	prev_opcode = cur_opcode - 1;

	/*
	if (cur_opcode->opcode == ZEND_QM_ASSIGN) {
#if PHP_VERSION_ID >= 70000
		ytrace_str_add(name, ytrace_sprintf("$%s", zend_get_compiled_variable_name(op_array, cur_opcode->result.var)->val), 1);
#else
		ytrace_str_add(name, ytrace_sprintf("$%s", zend_get_compiled_variable_name(op_array, cur_opcode->result.var, &cv_len)), 1);
#endif
	}
	*/

	if (cur_opcode->op1_type == IS_VAR &&
			(next_opcode->op1_type == IS_VAR || cur_opcode->op2_type == IS_VAR) &&
			prev_opcode->opcode == ZEND_FETCH_RW &&
			prev_opcode->op1_type == IS_CONST &&
#if PHP_VERSION_ID >= 70000
			Z_TYPE_P(RT_CONSTANT_EX(op_array->literals, prev_opcode->op1)) == IS_STRING
#else
			Z_TYPE_P(prev_opcode->op1.zv) == IS_STRING
#endif
	) {
#if PHP_VERSION_ID >= 70000
		ytrace_str_append(name, ytrace_sprintf("$%s", Z_STRVAL_P(RT_CONSTANT_EX(op_array->literals, prev_opcode->op1))), 1);
#else
		ytrace_str_append(name, ytrace_sprintf("$%s", Z_STRVAL_P(prev_opcode->op1.zv)), 1);
#endif
	}

	is_static = ytrace_is_static_call(cur_opcode, prev_opcode, &static_opcode_ptr);

	if (cur_opcode->op1_type == IS_CV) {
#if PHP_VERSION_ID >= 70000
		ytrace_str_append(name, ytrace_sprintf("$%s", zend_get_compiled_variable_name(op_array, cur_opcode->op1.var)->val), 1);
#else
		ytrace_str_append(name, ytrace_sprintf("$%s", zend_get_compiled_variable_name(op_array, cur_opcode->op1.var, &cv_len)), 1);
#endif
	} else if (cur_opcode->op1_type == IS_VAR && cur_opcode->opcode == ZEND_ASSIGN && (prev_opcode->opcode == ZEND_FETCH_W || prev_opcode->opcode == ZEND_FETCH_RW
#if PHP_VERSION_ID >= 70100
				|| prev_opcode->opcode == ZEND_FETCH_STATIC_PROP_W || prev_opcode->opcode == ZEND_FETCH_STATIC_PROP_RW
#endif
				)) {
		if (is_static) {
			if (prev_opcode->op2_type == IS_CONST) {
#if PHP_VERSION_ID >= 70000
				zend_class_entry *ce =  zend_fetch_class_by_name(Z_STR_P(EX_CONSTANT(prev_opcode->op2)), EX_CONSTANT(prev_opcode->op2) + 1, ZEND_FETCH_CLASS_SILENT);
				if (ce) {
					ytrace_str_append(name, ce->name->val, 0);
				}
#else
				ytrace_str_appendl(name, Z_STRVAL_P(prev_opcode->op2.zv), Z_STRLEN_P(prev_opcode->op2.zv), 0);
#endif
			} else {
#if PHP_VERSION_ID >= 70000
#if PHP_VERSION_ID >= 70100
				zend_class_entry *ce;
				if (prev_opcode->op2_type == IS_UNUSED) {
					ce = zend_fetch_class(NULL, prev_opcode->op2.num);
				} else {
					ce = Z_CE_P(EX_VAR(prev_opcode->op2.var));
				}
#else
				zend_class_entry *ce = Z_CE_P(EX_VAR(prev_opcode->op2.var));
#endif
				ytrace_str_append(name, ce->name->val, 0);
#else
				zend_class_entry *ce = EX_TMP_VAR(execute_data, prev_opcode->op2.var)->class_entry;
				ytrace_str_appendl(name, (char*)ce->name, ce->name_length, 0);
#endif
			}
			ytrace_str_append(name, "::", 0);
		} else {
			ytrace_get_zval_value(ytrace_get_zval(execute_data, prev_opcode->op1_type, &prev_opcode->op1), name);
		}
	} else if (is_static) { /* todo : see if you can change this and the previous cases around */
		ytrace_str_append(name, "selfAAA::", 0);
	}
	if ((cur_opcode->opcode >= ZEND_ASSIGN_ADD && cur_opcode->opcode <= ZEND_ASSIGN_BW_XOR)
#if PHP_VERSION_ID >= 50600
		|| cur_opcode->opcode == ZEND_ASSIGN_POW
#endif
	) {
		if (cur_opcode->extended_value == ZEND_ASSIGN_OBJ) {
			if (cur_opcode->op1_type == IS_UNUSED) {
				ytrace_str_append(name, "$this->", 0);
			} else {
				ytrace_str_append(name, "->", 0);
			}
			ytrace_get_zval_value(ytrace_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2), name);
		} else if (cur_opcode->extended_value == ZEND_ASSIGN_DIM) {
			ytrace_str_append(name, "[", 0);
			ytrace_get_zval_value(ytrace_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2), name);
			ytrace_str_append(name, "]", 0);
		}
	}
	if (cur_opcode->opcode >= ZEND_PRE_INC_OBJ && cur_opcode->opcode <= ZEND_POST_DEC_OBJ) {
		ytrace_str_append(name, "$this->", 0);
		ytrace_get_zval_value(ytrace_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2), name);
	}

	/* Scroll back to start of FETCHES */
	/* FIXME: See whether we can do this unroll looping only once - in is_static() */
	gohungfound = 0;
#if PHP_VERSION_ID >= 70000
	if (!is_static) {
#endif
		opcode_ptr = prev_opcode;
		while (opcode_ptr->opcode == ZEND_FETCH_DIM_W || opcode_ptr->opcode == ZEND_FETCH_OBJ_W || opcode_ptr->opcode == ZEND_FETCH_W || opcode_ptr->opcode == ZEND_FETCH_RW) {
			opcode_ptr = opcode_ptr - 1;
			gohungfound = 1;
		}
		opcode_ptr = opcode_ptr + 1;
#if PHP_VERSION_ID >= 70000
	} else { /* if we have a static method, we should already have found the first fetch */
		opcode_ptr = static_opcode_ptr;
		gohungfound = 1;
	}
#endif

	if (gohungfound) {
		do
		{
			if (opcode_ptr->op1_type == IS_UNUSED && opcode_ptr->opcode == ZEND_FETCH_OBJ_W) {
				ytrace_str_append(name, "$this", 0);
			}
			if (opcode_ptr->op1_type == IS_CV) {
#if PHP_VERSION_ID >= 70000
				ytrace_str_append(name, ytrace_sprintf("$%s", zend_get_compiled_variable_name(op_array, opcode_ptr->op1.var)->val), 1);
#else
				ytrace_str_append(name, ytrace_sprintf("$%s", zend_get_compiled_variable_name(op_array, opcode_ptr->op1.var, &cv_len)), 1);
#endif
			}
#if PHP_VERSION_ID >= 70100
			if (opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_W || opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_R || opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_RW) {
				ytrace_get_zval_value(ytrace_get_zval(execute_data, opcode_ptr->op1_type, &opcode_ptr->op1), name);
			}
#endif
			if (opcode_ptr->opcode == ZEND_FETCH_W) {
				ytrace_get_zval_value(ytrace_get_zval(execute_data, opcode_ptr->op1_type, &opcode_ptr->op1), name);
			}
			if (is_static && opcode_ptr->opcode == ZEND_FETCH_RW) {
				ytrace_get_zval_value(ytrace_get_zval(execute_data, opcode_ptr->op1_type, &opcode_ptr->op1), name);
			}
			if (opcode_ptr->opcode == ZEND_FETCH_DIM_W) {
#if PHP_VERSION_ID < 70000
				if (opcode_ptr->op2_type != IS_VAR) {
#endif
					ytrace_str_append(name, "[", 0);
					ytrace_get_zval_value(ytrace_get_zval(execute_data, opcode_ptr->op2_type, &opcode_ptr->op2), name);
					ytrace_str_append(name, "]", 0);
#if PHP_VERSION_ID < 70000
				} else {
					ytrace_str_append(name, "[???]" , 0);
				}
#endif
			} else if (opcode_ptr->opcode == ZEND_FETCH_OBJ_W) {
				ytrace_str_append(name, "->", 0);
				ytrace_get_zval_value(ytrace_get_zval(execute_data, opcode_ptr->op2_type, &opcode_ptr->op2), name);
			}
			opcode_ptr = opcode_ptr + 1;
		} while (opcode_ptr->opcode == ZEND_FETCH_DIM_W || opcode_ptr->opcode == ZEND_FETCH_OBJ_W || opcode_ptr->opcode == ZEND_FETCH_W || opcode_ptr->opcode == ZEND_FETCH_RW);
	}

	if (cur_opcode->opcode == ZEND_ASSIGN_OBJ) {
		if (cur_opcode->op1_type == IS_UNUSED) {
			ytrace_str_append(name, "$this", 0);
		}
		dimval = ytrace_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2);
		ytrace_str_append(name, ytrace_sprintf("->%s", Z_STRVAL_P(dimval)), 1);
	}

	if (cur_opcode->opcode == ZEND_ASSIGN_DIM) {
		if (next_opcode->opcode == ZEND_OP_DATA && cur_opcode->op2_type == IS_UNUSED) {
			ytrace_str_append(name, "[]", 0);
		} else {
			ytrace_str_append(name, "[", 0);
			ytrace_get_zval_value(ytrace_get_zval(execute_data, opcode_ptr->op2_type, &opcode_ptr->op2), name);
			ytrace_str_append(name, "]", 0);
		}
	}
}
/* }}} */

/* {{{ ytrace_assign_override_handler(int type, zend_execute_data *execute_data)
 */
static int ytrace_assign_override_handler(int type, zend_execute_data *execute_data)
{
	TSRMLS_FETCH();
	if (!YTRACE_G(trace_file) || YTRACE_G(in_eval)) {
		return ZEND_USER_OPCODE_DISPATCH;
	}

    int lineno;
    const zend_op *cur_opcode, *next_opcode;
	ytrace_str str;

#if PHP_VERSION_ID >= 70000
    zend_op_array *op_array = &execute_data->func->op_array;
    cur_opcode = execute_data->opline;
    lineno = cur_opcode->lineno;
#else
    zend_op_array *op_array = execute_data->op_array;
    cur_opcode = *EG(opline_ptr);
    lineno = cur_opcode->lineno;
#endif

	next_opcode = cur_opcode + 1;

	char *filename = STR_NAME_VAL(op_array->filename);
	if (!ytrace_should_trace(filename)) return ZEND_USER_OPCODE_DISPATCH;

	ytrace_str_new(&str);
	ytrace_str_append(&str, ytrace_sprintf("%s\t%d\tA\t%zu\t", filename, lineno, YTRACE_G(level)), 1);
	ytrace_find_var_name(&str, execute_data TSRMLS_CC);
	ytrace_str_append(&str, "\t", 0);

	if (type == ZEND_ASSIGN_OBJ || type == ZEND_ASSIGN_DIM) {
		ytrace_get_zval_value(ytrace_get_zval(execute_data, next_opcode->op1_type, &next_opcode->op1), &str);
	} else {
		if (type == ZEND_PRE_INC || type == ZEND_PRE_DEC || type == ZEND_POST_INC || type == ZEND_POST_DEC || type == ZEND_ASSIGN_ADD
				|| type == ZEND_ASSIGN_SUB || type == ZEND_ASSIGN_MUL || type == ZEND_ASSIGN_DIV || type == ZEND_ASSIGN_MOD
#if PHP_VERSION_ID >= 50600
				|| type == ZEND_ASSIGN_POW
#endif
				|| type == ZEND_ASSIGN_CONCAT || type == ZEND_ASSIGN_BW_OR || type == ZEND_ASSIGN_BW_AND || type == ZEND_ASSIGN_BW_XOR) {
			ytrace_get_zval_value(ytrace_get_zval(execute_data, cur_opcode->op1_type, &cur_opcode->op1), &str);
		} else {
			ytrace_get_zval_value(ytrace_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2), &str);
		}
	}
	ytrace_str_append(&str, "\n", 0);
	if (YTRACE_G(trace_file)) {
		fwrite(str.c, 1, str.len, YTRACE_G(trace_file));
	}
	ytrace_str_destroy(&str);

    return ZEND_USER_OPCODE_DISPATCH;
}
/* }}} */

#define YTRACE_OPCODE_OVERRIDE_ASSIGN(f, type) \
	int ytrace_##f##_handler(zend_execute_data *execute_data) \
	{ \
		return ytrace_assign_override_handler((type), execute_data); \
	}

YTRACE_OPCODE_OVERRIDE_ASSIGN(assign, ZEND_ASSIGN)
YTRACE_OPCODE_OVERRIDE_ASSIGN(assign_add, ZEND_ASSIGN_ADD)
YTRACE_OPCODE_OVERRIDE_ASSIGN(assign_sub, ZEND_ASSIGN_SUB)
YTRACE_OPCODE_OVERRIDE_ASSIGN(assign_mul, ZEND_ASSIGN_MUL)
YTRACE_OPCODE_OVERRIDE_ASSIGN(assign_div, ZEND_ASSIGN_DIV)
YTRACE_OPCODE_OVERRIDE_ASSIGN(assign_mod, ZEND_ASSIGN_MOD)
#if PHP_VERSION_ID >= 50600
YTRACE_OPCODE_OVERRIDE_ASSIGN(assign_pow, ZEND_ASSIGN_POW)
#endif
YTRACE_OPCODE_OVERRIDE_ASSIGN(assign_concat, ZEND_ASSIGN_CONCAT)
YTRACE_OPCODE_OVERRIDE_ASSIGN(assign_bw_or, ZEND_ASSIGN_BW_OR)
YTRACE_OPCODE_OVERRIDE_ASSIGN(assign_bw_and, ZEND_ASSIGN_BW_AND)
YTRACE_OPCODE_OVERRIDE_ASSIGN(assign_bw_xor, ZEND_ASSIGN_BW_XOR)
YTRACE_OPCODE_OVERRIDE_ASSIGN(assign_dim, ZEND_ASSIGN_DIM);
YTRACE_OPCODE_OVERRIDE_ASSIGN(assign_obj, ZEND_ASSIGN_OBJ);
YTRACE_OPCODE_OVERRIDE_ASSIGN(pre_inc, ZEND_PRE_INC)
YTRACE_OPCODE_OVERRIDE_ASSIGN(post_inc, ZEND_POST_INC)
YTRACE_OPCODE_OVERRIDE_ASSIGN(pre_dec, ZEND_PRE_DEC)
YTRACE_OPCODE_OVERRIDE_ASSIGN(post_dec, ZEND_POST_DEC)

#define YTRACE_SET_OPCODE_OVERRIDE_COMMON(oc) \
    zend_set_user_opcode_handler(oc, ytrace_common_override_handler);
#define YTRACE_SET_OPCODE_OVERRIDE_ASSIGN(f,oc) \
    zend_set_user_opcode_handler(oc, ytrace_##f##_handler);

/* {{{ ytrace_override_handler_init()
 */
void ytrace_override_handler_init()
{
	YTRACE_SET_OPCODE_OVERRIDE_ASSIGN(include_or_eval, ZEND_INCLUDE_OR_EVAL);
	YTRACE_SET_OPCODE_OVERRIDE_ASSIGN(assign, ZEND_ASSIGN);
	YTRACE_SET_OPCODE_OVERRIDE_ASSIGN(assign_add, ZEND_ASSIGN_ADD);
	YTRACE_SET_OPCODE_OVERRIDE_ASSIGN(assign_sub, ZEND_ASSIGN_SUB);
	YTRACE_SET_OPCODE_OVERRIDE_ASSIGN(assign_mul, ZEND_ASSIGN_MUL);
	YTRACE_SET_OPCODE_OVERRIDE_ASSIGN(assign_div, ZEND_ASSIGN_DIV);
	YTRACE_SET_OPCODE_OVERRIDE_ASSIGN(assign_mod, ZEND_ASSIGN_MOD);
#if PHP_VERSION_ID >= 50600
	YTRACE_SET_OPCODE_OVERRIDE_ASSIGN(assign_pow, ZEND_ASSIGN_POW);
#endif
	YTRACE_SET_OPCODE_OVERRIDE_ASSIGN(assign_concat, ZEND_ASSIGN_CONCAT);
	YTRACE_SET_OPCODE_OVERRIDE_ASSIGN(assign_bw_or, ZEND_ASSIGN_BW_OR);
	YTRACE_SET_OPCODE_OVERRIDE_ASSIGN(assign_bw_and, ZEND_ASSIGN_BW_AND);
	YTRACE_SET_OPCODE_OVERRIDE_ASSIGN(assign_bw_xor, ZEND_ASSIGN_BW_XOR);
	YTRACE_SET_OPCODE_OVERRIDE_ASSIGN(assign_dim, ZEND_ASSIGN_DIM);
	YTRACE_SET_OPCODE_OVERRIDE_ASSIGN(assign_obj, ZEND_ASSIGN_OBJ);
	YTRACE_SET_OPCODE_OVERRIDE_ASSIGN(pre_inc, ZEND_PRE_INC);
	YTRACE_SET_OPCODE_OVERRIDE_ASSIGN(post_inc, ZEND_POST_INC);
	YTRACE_SET_OPCODE_OVERRIDE_ASSIGN(pre_dec, ZEND_PRE_DEC);
	YTRACE_SET_OPCODE_OVERRIDE_ASSIGN(post_dec, ZEND_POST_DEC);
}
/* }}} */
