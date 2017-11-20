/**
 * Copyright 2017 Rokety Yang <yangrokety@gmail.com>
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

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_ytrace.h"
#include "SAPI.h"

#include "ytrace_opcode_handler_override.h"
#include "ytrace_execute.h"
#include "ytrace_file.h"

/* If you declare any globals in php_ytrace.h uncomment this:
*/
ZEND_DECLARE_MODULE_GLOBALS(ytrace)

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
	/* trace trigger settings */
	STD_PHP_INI_BOOLEAN("ytrace.auto_enable",		"0",		PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool, auto_enable,	 zend_ytrace_globals, ytrace_globals)
	STD_PHP_INI_BOOLEAN("ytrace.enable_trigger",	"0",		PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool, enable_trigger, zend_ytrace_globals, ytrace_globals)
	STD_PHP_INI_ENTRY("ytrace.enable_trigger_name", "YTRACE_TRIGGER",	PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, enable_trigger_name, zend_ytrace_globals, ytrace_globals)
	STD_PHP_INI_ENTRY("ytrace.enable_trigger_value", "",				PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, enable_trigger_value, zend_ytrace_globals, ytrace_globals)

	/* trace output settings */
	STD_PHP_INI_ENTRY("ytrace.output_dir",  "/tmp",				PHP_INI_SYSTEM|PHP_INI_PERDIR,	OnUpdateString, output_dir,			zend_ytrace_globals, ytrace_globals)
	STD_PHP_INI_ENTRY("ytrace.output_format", "trace-%t",		PHP_INI_SYSTEM|PHP_INI_PERDIR,	OnUpdateString, output_format,		zend_ytrace_globals, ytrace_globals)

	/* trace filter settings */
	STD_PHP_INI_ENTRY("ytrace.white_list",	"",	PHP_INI_ALL,	OnUpdateString, white_list,	zend_ytrace_globals, ytrace_globals)
	STD_PHP_INI_ENTRY("ytrace.white_list_name",	"YTRACE_WHITE_LIST",	PHP_INI_SYSTEM|PHP_INI_PERDIR,	OnUpdateString, white_list_name,	zend_ytrace_globals, ytrace_globals)
	STD_PHP_INI_ENTRY("ytrace.black_list",	"",	PHP_INI_ALL,    OnUpdateString, black_list,	zend_ytrace_globals, ytrace_globals)
	STD_PHP_INI_ENTRY("ytrace.black_list_name",	"YTRACE_BLACK_LIST",	PHP_INI_SYSTEM|PHP_INI_PERDIR,	OnUpdateString, black_list_name,	zend_ytrace_globals, ytrace_globals)

	/* variable display settings */
	STD_PHP_INI_ENTRY("ytrace.var_display_max_children", "128", PHP_INI_ALL,	OnUpdateLong,   display_max_children, zend_ytrace_globals, ytrace_globals)
	STD_PHP_INI_ENTRY("ytrace.var_display_max_data",     "512", PHP_INI_ALL,	OnUpdateLong,   display_max_data,     zend_ytrace_globals, ytrace_globals)
	STD_PHP_INI_ENTRY("ytrace.var_display_max_depth",    "3",   PHP_INI_ALL,	OnUpdateLong,   display_max_depth,    zend_ytrace_globals, ytrace_globals)
	STD_PHP_INI_ENTRY("ytrace.var_display_max_children_name", "YTRACE_VAR_DISPLAY_MAX_CHILDREN",	PHP_INI_SYSTEM|PHP_INI_PERDIR,	OnUpdateString,   display_max_children_name, zend_ytrace_globals, ytrace_globals)
	STD_PHP_INI_ENTRY("ytrace.var_display_max_data_name",     "YTRACE_VAR_DISPLAY_MAX_DATA",		PHP_INI_SYSTEM|PHP_INI_PERDIR,	OnUpdateString,   display_max_data_name,     zend_ytrace_globals, ytrace_globals)
	STD_PHP_INI_ENTRY("ytrace.var_display_max_depth_name",    "YTRACE_VAR_DISPLAY_MAX_DEPTH",		PHP_INI_SYSTEM|PHP_INI_PERDIR,	OnUpdateString,   display_max_depth_name,    zend_ytrace_globals, ytrace_globals)
PHP_INI_END()
/* }}} */

/* {{{ ytrace_init_auto_globals
 */
static void ytrace_init_auto_globals(TSRMLS_D)
{
	YTRACE_AUTO_GLOBAL("_ENV");
	YTRACE_AUTO_GLOBAL("_GET");
	YTRACE_AUTO_GLOBAL("_POST");
	YTRACE_AUTO_GLOBAL("_COOKIE");
	YTRACE_AUTO_GLOBAL("_REQUEST");
	YTRACE_AUTO_GLOBAL("_SERVER");
}
/* }}} */

#define YTRACE_GET_STR_VAL_FROM_HTTP_DATA_OR_ENV(var) \
	val = ytrace_get_val_from_http_data(YTRACE_G(var##_name)); \
	if (val) { \
		YTRACE_G(var) = val; \
		YTRACE_G(_##var) = val; \
	} else { \
		val = getenv(YTRACE_G(var##_name)); \
		if (val) { \
			YTRACE_G(var) = val; \
		} \
	}

#define YTRACE_GET_INT_VAL_FROM_HTTP_DATA_OR_ENV(var) \
	val = ytrace_get_val_from_http_data(YTRACE_G(var##_name)); \
	if (val) { \
		YTRACE_G(var) = atoi(val); \
		free(val); \
	} else { \
		val = getenv(YTRACE_G(var##_name)); \
		if (val) { \
			YTRACE_G(var) = atoi(val); \
		} \
	}

/* {{{ ytrace_enable(char *fname, size_t fname_len)
 */
void ytrace_enable(char *fname, size_t fname_len)
{
	if (YTRACE_G(trace_file)) {
		return;
	}

	ytrace_init_auto_globals(TSRMLS_C);

	ytrace_str filename;
	ytrace_str_new(&filename);
	if (fname_len > 0) {
		ytrace_str_appendl(&filename, fname, fname_len, 0);
	} else {
		ytrace_str_append(&filename, YTRACE_G(output_dir), 0);
		ytrace_format_output_filename(&filename, YTRACE_G(output_format));
	}
	YTRACE_G(trace_file) = fopen(filename.c, "w");
	if (YTRACE_G(trace_file) == NULL) {
		php_error(E_ERROR, "Open trace file %s failed!", filename.c);
	}
	ytrace_str_destroy(&filename);
	/* trace file info */
	fwrite(YTRACE_G(sapi), 1, strlen(YTRACE_G(sapi)), YTRACE_G(trace_file));
	if (strcmp(YTRACE_G(sapi), "fpm-fcgi") == 0) {
		char *val = ytrace_get_val_from_server("REQUEST_METHOD");
		if (val) {
			fwrite("\t", 1, strlen("\t"), YTRACE_G(trace_file));
			fwrite(val, 1, strlen(val), YTRACE_G(trace_file));
			free(val);
		}
		val = ytrace_get_val_from_server("REQUEST_URI");
		if (val) {
			fwrite("\t", 1, strlen("\t"), YTRACE_G(trace_file));
			fwrite(val, 1, strlen(val), YTRACE_G(trace_file));
			free(val);
		}
	}
	fwrite("\n", 1, strlen("\n"), YTRACE_G(trace_file));

	char *val;

	YTRACE_GET_STR_VAL_FROM_HTTP_DATA_OR_ENV(white_list)
	YTRACE_GET_STR_VAL_FROM_HTTP_DATA_OR_ENV(black_list)

	YTRACE_GET_INT_VAL_FROM_HTTP_DATA_OR_ENV(display_max_depth)
	YTRACE_GET_INT_VAL_FROM_HTTP_DATA_OR_ENV(display_max_data)
	YTRACE_GET_INT_VAL_FROM_HTTP_DATA_OR_ENV(display_max_children)
}
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
PHP_FUNCTION(ytrace_enable)
{
	char *fname = NULL;
	size_t fname_len = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &fname, &fname_len) == FAILURE) {
		return;
	}
	ytrace_enable(fname, fname_len);
}

PHP_FUNCTION(ytrace_disable)
{
	if (YTRACE_G(trace_file)) {
		fflush(YTRACE_G(trace_file));
		fclose(YTRACE_G(trace_file));
		YTRACE_G(trace_file) = NULL;
	}
}
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_ytrace_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_ytrace_init_globals(zend_ytrace_globals *ytrace_globals)
{
	ytrace_globals->global_value = 0;
	ytrace_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(ytrace)
{
	REGISTER_INI_ENTRIES();

	YTRACE_G(sapi) = sapi_module.name;

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

	YTRACE_G(var_export_runtime) = (ytrace_var_runtime*)malloc((YTRACE_G(display_max_depth) + 1) * sizeof(ytrace_var_runtime));

	/* hook execute */
	ytrace_old_execute_ex = zend_execute_ex;
	zend_execute_ex = ytrace_execute_ex;

	ytrace_old_execute_internal = zend_execute_internal;
	zend_execute_internal = ytrace_execute_internal;

	/* hook assign/include handler */
	ytrace_override_handler_init();

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(ytrace)
{
	UNREGISTER_INI_ENTRIES();
	free(YTRACE_G(var_export_runtime));
	return SUCCESS;
}
/* }}} */

/* {{{ ytrace_trigger_enabled(char *var_name, char *var_value TSRMLS_DC)
 */
static int ytrace_trigger_enabled(char *var_name, char *var_value TSRMLS_DC)
{
	if (!YTRACE_G(enable_trigger)) return 0;

	char *value = ytrace_get_val_from_http_data(var_name);
	zend_bool enable = 0;
	if (value && ((var_value == NULL) || (var_value[0] == '\0') ||
		(strcmp(var_value, value) == 0))
	) {
		enable = 1;
	}

	// detect from env
	char *env_trigger_val = getenv(var_name);
	if (env_trigger_val && *env_trigger_val && ((var_value == NULL) || (var_value[0] == '\0') || (strcmp(var_value, env_trigger_val) == 0))) {
		enable = 1;
	}

	if (value) {
		free(value);
	}

	return enable;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(ytrace)
{
	YTRACE_G(level) = 0;
	YTRACE_G(in_eval) = 0;
	if (YTRACE_G(auto_enable) || ytrace_trigger_enabled(YTRACE_G(enable_trigger_name), YTRACE_G(enable_trigger_value))) {
		ytrace_enable(NULL, 0);
	}
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(ytrace)
{
	if (YTRACE_G(trace_file)) {
		fflush(YTRACE_G(trace_file));
		fclose(YTRACE_G(trace_file));
		YTRACE_G(trace_file) = NULL;
	}

	if (YTRACE_G(_white_list)) {
		free(YTRACE_G(_white_list));
		YTRACE_G(_white_list) = NULL;
	}
	if (YTRACE_G(_black_list)) {
		free(YTRACE_G(_black_list));
		YTRACE_G(_black_list) = NULL;
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(ytrace)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "ytrace support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(ytrace_enable_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, fname)
ZEND_END_ARG_INFO()

/* {{{ ytrace_functions[]
 *
 * Every user visible function must have an entry in ytrace_functions[].
 */
const zend_function_entry ytrace_functions[] = {
	PHP_FE(ytrace_enable,	ytrace_enable_args)		/* For testing, remove later. */
	PHP_FE(ytrace_disable,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in ytrace_functions[] */
};
/* }}} */

/* {{{ ytrace_module_entry
 */
zend_module_entry ytrace_module_entry = {
	STANDARD_MODULE_HEADER,
	"ytrace",
	ytrace_functions,
	PHP_MINIT(ytrace),
	PHP_MSHUTDOWN(ytrace),
	PHP_RINIT(ytrace),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(ytrace),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(ytrace),
	PHP_YTRACE_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_YTRACE
ZEND_GET_MODULE(ytrace)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
