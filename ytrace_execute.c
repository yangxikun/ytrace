/**
 * Copyright 2017 Qihoo 360
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include "php.h"
#include "php_ytrace.h"
#include "ytrace_compat.h"
#include "ytrace_execute.h"
#include "ytrace_filter.h"

ZEND_EXTERN_MODULE_GLOBALS(ytrace)

#if PHP_VERSION_ID >= 70000

void (*ytrace_old_execute_ex)(zend_execute_data *execute_data TSRMLS_DC);
void (*ytrace_old_execute_internal)(zend_execute_data *current_execute_data, zval *return_value);

#else

void (*ytrace_old_execute_ex)(zend_execute_data *execute_data TSRMLS_DC);
void (*ytrace_old_execute_internal)(zend_execute_data *current_execute_data, zend_fcall_info *fci, int return_value_used TSRMLS_DC);

#endif

/* {{{ function_entry(zend_execute_data *ex TSRMLS_DC)
 */
void function_entry(zend_bool internal, zend_execute_data *ex TSRMLS_DC)
{
    unsigned int i	 = 0;
	int arg_count = 0, arg_wanted = 0;
	zval **args		 = NULL;

	ytrace_str str;
	zend_function *zf;
	zend_op_array *op_array;

#if PHP_VERSION_ID < 70000
    zf = ex->function_state.function;
#else
    zf = ex->func;
#endif

	if (zf->common.function_name) {
		/* filename */
#if PHP_VERSION_ID < 70000
		op_array = ex->op_array;
#else
		op_array = &(ex->prev_execute_data->func->op_array);
		if (!op_array || (op_array->type == 1)) {
			return;
		}
#endif
		char *filename = STR_NAME_VAL(op_array->filename);

		if (!ytrace_should_trace(filename)) return;
		ytrace_str_new(&str);

		ytrace_str_append(&str, filename, 0);
		ytrace_str_append(&str, "\t", 0);

#if PHP_VERSION_ID < 70000
		ytrace_str_append(&str, ytrace_sprintf("%zu\tF\t%zu\t", ex->opline->lineno, YTRACE_G(level)), 1);
#else
		if (internal) {
			ytrace_str_append(&str, ytrace_sprintf("%zu\tF\t%zu\t", zend_get_executed_lineno(), YTRACE_G(level)), 1);
		} else {
			if (!ex->prev_execute_data->opline || !ex->prev_execute_data->func || !ZEND_USER_CODE(ex->prev_execute_data->func->type)) {
				return;
			}
			ytrace_str_append(&str, ytrace_sprintf("%zu\tF\t%zu\t", ex->prev_execute_data->opline->lineno, YTRACE_G(level)), 1);
		}
#endif

		if (YTRACE_EX_OBJ(ex)) {
			if (zf->common.scope) {
				// obj method
				ytrace_str_append(&str, ytrace_sprintf("%s->", STR_NAME_VAL(zf->common.scope->name)), 1);
			}
		} else if (zf->common.scope) {
			// static method
			ytrace_str_append(&str, ytrace_sprintf("%s::", STR_NAME_VAL(zf->common.scope->name)), 1);
		}

		if (strcmp(STR_NAME_VAL(zf->common.function_name), "{closure}") == 0) {
			ytrace_str_append(&str, "{closure}", 0);
		} else if (strcmp(STR_NAME_VAL(zf->common.function_name), "__lambda_func") == 0) {
			ytrace_str_append(&str, "{lambda}", 0);
#if PHP_VERSION_ID >= 50414
		} else if (zf->common.scope && zf->common.scope->trait_aliases) {
			/* Use trait alias instead real function name.
			 * There is also a bug "#64239 Debug backtrace changed behavior
			 * since 5.4.10 or 5.4.11" about this
			 * https://bugs.php.net/bug.php?id=64239.*/
			//frame->function = sdsnew(P7_STR(zend_resolve_method_name(P7_EX_OBJ(ex) ? P7_EX_OBJCE(ex) : zf->common.scope, zf)));
			ytrace_str_append(&str, STR_NAME_VAL(zend_resolve_method_name(YTRACE_EX_OBJ(ex) ? YTRACE_EX_OBJCE(ex) : zf->common.scope, zf)), 0);
#endif
		} else {
			ytrace_str_append(&str, STR_NAME_VAL(zf->common.function_name), 0);
		}

#if PHP_VERSION_ID < 70000
		if (ex && ex->function_state.arguments) {
			arg_count = (int)(zend_uintptr_t) *(ex->function_state.arguments);
			args = (zval **)(ex->function_state.arguments - arg_count);
		}
#else
		arg_count = ZEND_CALL_NUM_ARGS(ex);
#endif

		ytrace_str_append(&str, ytrace_sprintf("\t%zu", arg_count), 1);

#if PHP_VERSION_ID < 70000
		arg_wanted = zf->common.num_args < arg_count ? zf->common.num_args : arg_count;

        for (i = 0; i < arg_count; i++) {
			if (i < arg_wanted) {
				ytrace_str_append(&str, ytrace_sprintf("\t$%s\t", zf->common.arg_info[i].name), 1);
			} else {
				ytrace_str_append(&str, "\t$...\t", 0);
			}
			ytrace_get_zval_value(args[i], &str);
        }
#else
		if (!internal) {
			op_array = &(ex->func->op_array);
		}
		if (ZEND_USER_CODE(zf->type)) {
			arg_wanted = op_array->num_args;
		}
		/* PHP 7 */
        if (arg_count) {
            i = 0;
            zval *p = ZEND_CALL_ARG(ex, 1);
			/*
            if (ex->func->type == ZEND_USER_FUNCTION) {
                uint32_t first_extra_arg = ex->func->op_array.num_args;

                if (first_extra_arg && arg_count > first_extra_arg) {
                    while (i < first_extra_arg) {
						ytrace_str_append(&str, "\t", 0);
						ytrace_get_zval_value(p++, &str);
                    }
                    p = ZEND_CALL_VAR_NUM(ex, ex->func->op_array.last_var + ex->func->op_array.T);
                }
            }
			*/
            while(i < arg_count) {
				if (i < arg_wanted) {
					ytrace_str_append(&str, ytrace_sprintf("\t$%s\t", STR_NAME_VAL(op_array->arg_info[i].name)), 1);
				} else {
					ytrace_str_append(&str, "\t$...\t", 0);
				}
				ytrace_get_zval_value(p++, &str);
				i++;
            }
        }
#endif
		ytrace_str_append(&str, "\n", 0);
		if (YTRACE_G(trace_file)) {
			fwrite(str.c, 1, str.len, YTRACE_G(trace_file));
		}
		ytrace_str_destroy(&str);
	}
}
/* }}} */

/* {{{ ytrace_execute_ex(zend_execute_data *execute_data TSRMLS_DC)
 */
void ytrace_execute_ex(zend_execute_data *execute_data TSRMLS_DC)
{
	YTRACE_G(level) += 1;
    zend_execute_data *ex = execute_data;
#if PHP_VERSION_ID < 70000
    if (execute_data->prev_execute_data) {
        ex = execute_data->prev_execute_data;
    }
	if (ex->op_array && YTRACE_G(trace_file)) {
#else
	if (YTRACE_G(trace_file)) {
#endif
		function_entry(0, ex TSRMLS_CC);
	}

	ytrace_old_execute_ex(execute_data TSRMLS_CC);
	YTRACE_G(level) -= 1;
	YTRACE_G(in_eval) = 0;
}
/* }}} */

/* {{{ ytrace_execute_internal(zend_execute_data *execute_data, zend_fcall_info *fci, int return_value_used TSRMLS_DC)
 */
#if PHP_VERSION_ID < 70000
void ytrace_execute_internal(zend_execute_data *execute_data, zend_fcall_info *fci, int return_value_used TSRMLS_DC)
{
#else
void ytrace_execute_internal(zend_execute_data *execute_data, zval *return_value)
{
#endif
	YTRACE_G(level) += 1;
#if PHP_VERSION_ID < 70000
	if (execute_data->op_array && YTRACE_G(trace_file)) {
#else
	if (YTRACE_G(trace_file)) {
#endif
		function_entry(1, execute_data TSRMLS_CC);
	}

	if (ytrace_old_execute_internal) {
#if PHP_VERSION_ID < 70000
		ytrace_old_execute_internal(execute_data, fci, return_value_used TSRMLS_CC);
#else
		ytrace_old_execute_internal(execute_data, return_value TSRMLS_CC);
#endif
	} else {
#if PHP_VERSION_ID < 70000
		execute_internal(execute_data, fci, return_value_used TSRMLS_CC);
#else
		execute_internal(execute_data, return_value TSRMLS_CC);
#endif
	}
	YTRACE_G(level) -= 1;
}
/* }}} */
