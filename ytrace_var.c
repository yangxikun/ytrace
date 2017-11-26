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
   | Authors:  Derick Rethans <derick@xdebug.org>                         |
   +----------------------------------------------------------------------+
 */
#include <stdlib.h>
#include "php.h"
#include "php_ytrace.h"
#include "ytrace_compat.h"

ZEND_EXTERN_MODULE_GLOBALS(ytrace)

#if PHP_VERSION_ID >= 70000
static int ytrace_array_element_export(zval *zv_nptr, zend_ulong index_key, zend_string *hash_key, ytrace_str *str, int level)
{
	zval **zv = &zv_nptr;
#else
static int ytrace_array_element_export(zval **zv TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	ytrace_str *str = va_arg(args, ytrace_str*);
	int level = va_arg(args, int);
#endif
	if (YTRACE_G(var_export_runtime)[level].current < YTRACE_G(display_max_children)) {
		ytrace_str_appendl(str, ytrace_str_tab_pad(level), level, 1);
		if (HASH_KEY_IS_NUMERIC(hash_key)) { /* numeric key */
#if PHP_VERSION_ID >= 70000
			ytrace_str_append(str, ytrace_sprintf(YTRACE_INT_FMT " => ", index_key), 1);
#else
			ytrace_str_append(str, ytrace_sprintf(YTRACE_INT_FMT " => ", HASH_APPLY_NUMERIC(hash_key)), 1);
#endif
		} else { /* string key */
			int tmp_len;
			char *tmp_str;
			tmp_str = ytrace_str_addcslashes((char*) HASH_APPLY_KEY_VAL(hash_key), HASH_APPLY_KEY_LEN(hash_key) - 1, &tmp_len);

			ytrace_str_append(str, "'", 0);
			ytrace_str_appendl(str, tmp_str, tmp_len, 0);
			ytrace_str_append(str, "' => ", 0);
			efree(tmp_str);
		}
		ytrace_var_export(zv, str, level + 1 TSRMLS_CC);
		ytrace_str_append(str, ",\n", 0);
	}
	if (YTRACE_G(var_export_runtime)[level].current == YTRACE_G(display_max_children)) {
		ytrace_str_appendl(str, ytrace_str_tab_pad(level), level, 1);
		ytrace_str_append(str, "...\n", 0);
	}
	YTRACE_G(var_export_runtime)[level].current++;
	return 0;
}

/* {{{ ytrace_get_property_info
 */
static char *ytrace_get_property_info(char *mangled_property, int mangled_len, char **property_name, char **class_name)
{
	const char *prop_name, *cls_name;

#if PHP_VERSION_ID >= 70000
	zend_string *i_mangled = zend_string_init(mangled_property, mangled_len - 1, 0);
	zend_unmangle_property_name(i_mangled, &cls_name, &prop_name);
#else
	zend_unmangle_property_name(mangled_property, mangled_len, &cls_name, &prop_name);
#endif
	*property_name = (char *) strdup(prop_name);
	*class_name = cls_name ? strdup(cls_name) : NULL;
#if PHP_VERSION_ID >= 70000
	zend_string_release(i_mangled);
#endif
	if (*class_name) {
		if (*class_name[0] == '*') {
			return "protected";
		} else {
			return "private";
		}
	} else {
		return "public";
	}
}
/* }}} */

#if PHP_VERSION_ID >= 70000
static int ytrace_object_element_export(zval *zv_nptr, zend_ulong index_key, zend_string *hash_key, ytrace_str *str, char *class_name, int level)
{
	zval **zv = &zv_nptr;
#else
static int ytrace_object_element_export(zval **zv TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	ytrace_str *str = va_arg(args, ytrace_str*);
	char *class_name = va_arg(args, char*);
	int level = va_arg(args, int);
#endif
	if (YTRACE_G(var_export_runtime)[level].current < YTRACE_G(display_max_children)) {
		ytrace_str_appendl(str, ytrace_str_tab_pad(level), level, 1);

		if (!HASH_KEY_IS_NUMERIC(hash_key)) {
			char *prop_name, *modifier, *prop_class_name;

			modifier = ytrace_get_property_info((char*) HASH_APPLY_KEY_VAL(hash_key), HASH_APPLY_KEY_LEN(hash_key), &prop_name, &prop_class_name);
			if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
				ytrace_str_append(str, ytrace_sprintf("%s $%s = ", modifier, prop_name), 1);
			} else {
				ytrace_str_append(str, ytrace_sprintf("%s ${%s}:%s = ", modifier, prop_class_name, prop_name), 1);
			}

			free(prop_name);
			free(prop_class_name);
		} else {
#if PHP_VERSION_ID >= 70000
			ytrace_str_append(str, ytrace_sprintf("public $%d = ", index_key), 1);
#else
			ytrace_str_append(str, ytrace_sprintf("public $%d = ", HASH_APPLY_NUMERIC(hash_key)), 1);
#endif
		}
		ytrace_var_export(zv, str, level + 1 TSRMLS_CC);
		ytrace_str_append(str, ";\n", 0);
	}
	if (YTRACE_G(var_export_runtime)[level].current == YTRACE_G(display_max_children)) {
		ytrace_str_appendl(str, ytrace_str_tab_pad(level), level, 1);
		ytrace_str_append(str, "...\n", 0);
	}
	YTRACE_G(var_export_runtime)[level].current++;
	return 0;
}

/* {{{ ytrace_get_property_info
 */
void ytrace_var_export(zval **struc, ytrace_str *str, int level TSRMLS_DC)
{
	HashTable *myht;
	char *tmp_str;
	int is_temp = 0;
#if PHP_VERSION_ID >= 70000
	zend_ulong num;
	zend_string *key;
	zval *val;
	zval *tmpz;
#endif

	if (!struc || !(*struc)) {
		return;
	}

#if PHP_VERSION_ID >= 70000
	if (Z_TYPE_P(*struc) == IS_REFERENCE) {
		tmpz = &((*struc)->value.ref->val);
		struc = &tmpz;
	}
#endif

	switch (Z_TYPE_P(*struc)) {
#if PHP_VERSION_ID >= 70000
		case IS_TRUE:
		case IS_FALSE:
			ytrace_str_append(str, ytrace_sprintf("%s", Z_TYPE_P(*struc) == IS_TRUE ? "TRUE" : "FALSE"), 1);
			break;
#else
		case IS_BOOL:
			ytrace_str_append(str, ytrace_sprintf("%s", Z_LVAL_P(*struc) ? "TRUE" : "FALSE"), 1);
			break;
#endif

		case IS_NULL:
			ytrace_str_append(str, "NULL", 0);
			break;

		case IS_LONG:
			ytrace_str_append(str, ytrace_sprintf(YTRACE_INT_FMT, Z_LVAL_P(*struc)), 1);
			break;

		case IS_DOUBLE:
			ytrace_str_append(str, ytrace_sprintf("%.*G", (int) EG(precision), Z_DVAL_P(*struc)), 1);
			break;

		case IS_STRING: {
			int tmp_len;
			tmp_str = ytrace_str_addcslashes(Z_STRVAL_P(*struc), Z_STRLEN_P(*struc), &tmp_len);
			ytrace_str_append(str, "'", 0);
			if (Z_STRLEN_P(*struc) < YTRACE_G(display_max_data)) {
				ytrace_str_appendl(str, tmp_str, tmp_len, 0);
			} else {
				ytrace_str_appendl(str, tmp_str, YTRACE_G(display_max_data), 0);
				ytrace_str_append(str, "...", 0);
			}
			ytrace_str_append(str, "'", 0);
			efree(tmp_str);
		} break;

		case IS_ARRAY:
			myht = Z_ARRVAL_P(*struc);
			if (YTRACE_APPLY_COUNT(myht) < 1) {
				if (myht->nNumOfElements == 0) {
					ytrace_str_append(str, "array ()", 0);
					break;
				}
				if (level > YTRACE_G(display_max_depth)) {
					ytrace_str_append(str, "array (...)", 0);
					break;
				}
				ytrace_str_append(str, "array (\n", 0);

				YTRACE_G(var_export_runtime)[level + 1].current = 0;

#if PHP_VERSION_ID >= 70000
				ZEND_HASH_INC_APPLY_COUNT(myht);
				ZEND_HASH_FOREACH_KEY_VAL_IND(myht, num, key, val) {
					ytrace_array_element_export(val, num, key, str, level + 1);
				} ZEND_HASH_FOREACH_END();
				ZEND_HASH_DEC_APPLY_COUNT(myht);
#else
				zend_hash_apply_with_arguments(myht TSRMLS_CC, (apply_func_args_t) ytrace_array_element_export, 2, str, level + 1);
#endif
				/* Remove the ", " at the end of the string */
				ytrace_str_chop(str, 2);
				if (level > 0) {
					ytrace_str_append(str, "\n", 0);
					ytrace_str_appendl(str, ytrace_str_tab_pad(level - 1), level - 1, 1);
					ytrace_str_append(str, ")", 0);
				} else {
					ytrace_str_append(str, "\n)", 0);
				}
			} else {
				ytrace_str_append(str, "...", 0);
			}
			break;

		case IS_OBJECT:
			if (Z_OBJ_HANDLER_P(*struc, get_debug_info)) {
				myht = Z_OBJ_HANDLER_P(*struc, get_debug_info)(*struc, &is_temp TSRMLS_CC);
			} else if (Z_OBJ_HANDLER_P(*struc, get_properties)) {
				myht =  Z_OBJPROP_P(*struc);
			} else {
				break;
			}
			if (YTRACE_APPLY_COUNT(myht) < 1) {
				char *class_name = (char*) STR_NAME_VAL(Z_OBJCE_P(*struc)->name);
				if (myht->nNumOfElements == 0) {
					ytrace_str_append(str, ytrace_sprintf("class %s {}", class_name), 1);
					break;
				}
				if (level > YTRACE_G(display_max_depth)) {
					ytrace_str_append(str, ytrace_sprintf("class %s {...}", class_name), 1);
					break;
				}
				ytrace_str_append(str, ytrace_sprintf("class %s {\n", class_name), 1);

				YTRACE_G(var_export_runtime)[level + 1].current = 0;

#if PHP_VERSION_ID >= 70000
				ZEND_HASH_INC_APPLY_COUNT(myht);
				ZEND_HASH_FOREACH_KEY_VAL_IND(myht, num, key, val) {
					ytrace_object_element_export(val, num, key, str, class_name, level + 1);
				} ZEND_HASH_FOREACH_END();
				ZEND_HASH_DEC_APPLY_COUNT(myht);
#else
				zend_hash_apply_with_arguments(myht TSRMLS_CC, (apply_func_args_t) ytrace_object_element_export, 2, str, class_name, level + 1);
#endif
				/* Remove the ", " at the end of the string */
				ytrace_str_chop(str, 2);
				if (level > 0) {
					ytrace_str_append(str, "\n", 0);
					ytrace_str_appendl(str, ytrace_str_tab_pad(level - 1), level - 1, 1);
					ytrace_str_append(str, "}", 0);
				} else {
					ytrace_str_append(str, "\n}", 0);
				}
			} else {
				ytrace_str_append(str, "...", 0);
			}
			if (is_temp) {
				zend_hash_destroy(myht);
				efree(myht);
			}
			break;

		case IS_RESOURCE: {
			char *type_name;

#if PHP_VERSION_ID >= 70000
			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_RES_P(*struc) TSRMLS_CC);
			ytrace_str_append(str, ytrace_sprintf("resource(%ld) of type (%s)", Z_RES_P(*struc)->handle, type_name ? type_name : "Unknown"), 1);
#else
			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_LVAL_P(*struc) TSRMLS_CC);
			ytrace_str_append(str, ytrace_sprintf("resource(%ld) of type (%s)", Z_LVAL_P(*struc), type_name ? type_name : "Unknown"), 1);
#endif
			break;
		}

#if PHP_VERSION_ID >= 70000
		case IS_UNDEF:
			ytrace_str_append(str, "*uninitialized*", 0);
			break;
#endif

		default:
			ytrace_str_append(str, "NFC", 0);
			break;
	}
}
/* }}} */

#if PHP_VERSION_ID >= 70000
# define T(offset) (*(union _temp_variable *)((char*)zdata->current_execute_data->Ts + offset))
#elif PHP_VERSION_ID >= 50500
# define T(offset) (*EX_TMP_VAR(zdata, offset))
#else
# define T(offset) (*(temp_variable *)((char*)zdata->Ts + offset))
#endif

#if PHP_VERSION_ID >= 70000
zval *ytrace_get_zval(zend_execute_data *zdata, int node_type, const znode_op *node)
{
	zend_free_op should_free;

	return zend_get_zval_ptr(node_type, node, zdata, &should_free, BP_VAR_IS);
}
#else
zval *ytrace_get_zval(zend_execute_data *zdata, int node_type, const znode_op *node)
{
	switch (node_type) {
		case IS_CONST:
#if PHP_VERSION_ID >= 50300
			return node->zv;
#else
			return &node->u.constant;
#endif
			break;

		case IS_TMP_VAR:
			return &T(node->var).tmp_var;
			break;

		case IS_VAR:
			if (T(node->var).var.ptr) {
				return T(node->var).var.ptr;
			}
			break;

		case IS_CV: {
			zval **tmp;
			tmp = zend_get_compiled_variable_value(zdata, node->constant);
			if (tmp) {
				return *tmp;
			}
			break;
		}

		case IS_UNUSED:
			fprintf(stderr, "\nIS_UNUSED\n");
			break;

		default:
			fprintf(stderr, "\ndefault %d\n", node_type);
			break;
	}

	return NULL;
}
#endif

void ytrace_get_zval_value(zval *val, ytrace_str *str)
{
	int tmp_len;
	char *tmp_str;
	ytrace_str zval_str;
	ytrace_str_new(&zval_str);

	/* init variable display settings */
	if (YTRACE_G(display_max_children) > 32) {
		YTRACE_G(display_max_children) = 32;
	} else if (YTRACE_G(display_max_children) < 1) {
		YTRACE_G(display_max_children) = 0;
	}

	if (YTRACE_G(display_max_data) > 1024) {
		YTRACE_G(display_max_data) = 1024;
	} else if (YTRACE_G(display_max_data) < 1) {
		YTRACE_G(display_max_data) = 0;
	}

	if (YTRACE_G(display_max_depth) > 16) {
		YTRACE_G(display_max_depth) = 16;
	} else if (YTRACE_G(display_max_depth) < 1) {
		YTRACE_G(display_max_depth) = 0;
	}

	ytrace_var_export(&val, &zval_str, 0 TSRMLS_CC);
	tmp_str = ytrace_str_addcslashes(zval_str.c, zval_str.len, &tmp_len);
	ytrace_str_appendl(str, tmp_str, tmp_len, 0);
	efree(tmp_str);
	ytrace_str_destroy(&zval_str);
}

char* ytrace_get_val_from_server(char *var_name)
{
#if PHP_VERSION_ID >= 70000
	zval *val = NULL;

	if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) == IS_ARRAY) {
		val = zend_hash_str_find(Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]), var_name, strlen(var_name));

		if (val) {
			return strndup(Z_STRVAL_P(val), Z_STRLEN_P(val));
		}
	}
#else
	int retval = FAILURE;
	zval **val;

	if (PG(http_globals)[TRACK_VARS_SERVER]) {
		retval = zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), var_name, strlen(var_name) + 1, (void **) &val);

		if (retval == SUCCESS) {
			return strndup(Z_STRVAL_PP(val), Z_STRLEN_PP(val));
		}
	}
#endif

	return NULL;
}

char* ytrace_get_val_from_http_data(char *var_name)
{
#if PHP_VERSION_ID >= 70000
	zval *val;
#else
	zval **val;
#endif
	if (
		(
#if PHP_VERSION_ID >= 70000
			(
				(val = zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_GET]), var_name, strlen(var_name))) != NULL
			) || (
				(val = zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_POST]), var_name, strlen(var_name))) != NULL
			) || (
				(val = zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_COOKIE]), var_name, strlen(var_name))) != NULL
			)
#else
			(
				PG(http_globals)[TRACK_VARS_GET] &&
				zend_hash_find(PG(http_globals)[TRACK_VARS_GET]->value.ht, var_name, strlen(var_name) + 1, (void **) &val) == SUCCESS
			) || (
				PG(http_globals)[TRACK_VARS_POST] &&
				zend_hash_find(PG(http_globals)[TRACK_VARS_POST]->value.ht, var_name, strlen(var_name) + 1, (void **) &val) == SUCCESS
			) || (
				PG(http_globals)[TRACK_VARS_COOKIE] &&
				zend_hash_find(PG(http_globals)[TRACK_VARS_COOKIE]->value.ht, var_name, strlen(var_name) + 1, (void **) &val) == SUCCESS
			)
#endif
		)
	) {
#if PHP_VERSION_ID >= 70000
			return strndup(Z_STRVAL_P(val), Z_STRLEN_P(val));
#else
			return strndup(Z_STRVAL_PP(val), Z_STRLEN_PP(val));
#endif
	}

	return NULL;
}
