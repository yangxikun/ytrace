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

#ifndef PHP_YTRACE_H
#define PHP_YTRACE_H

extern zend_module_entry ytrace_module_entry;
#define phpext_ytrace_ptr &ytrace_module_entry

#define PHP_YTRACE_VERSION "0.1.0" /* Replace with version number for your extension */

#include "zend_compile.h"
#include "ytrace_compat.h"
#include "ytrace_var.h"

#ifdef PHP_WIN32
#	define PHP_YTRACE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_YTRACE_API __attribute__ ((visibility("default")))
#else
#	define PHP_YTRACE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     
*/

ZEND_BEGIN_MODULE_GLOBALS(ytrace)
	unsigned long level;
	FILE *trace_file;

	zend_bool	in_eval;

	char		*sapi;

	/* trace trigger settings */
	zend_bool	auto_enable;
	zend_bool	enable_trigger;
	char		*enable_trigger_name;
	char		*enable_trigger_value;

	/* trace output settings */
	char        *output_dir;
	char        *output_format;
	char        *output_name;

	/* trace filter settings */
	char		*white_list;
	char		*_white_list; // for free
	char		*white_list_name;
	char		*black_list;
	char		*_black_list;
	char		*black_list_name;

	/* Variable display settings */
	iniLONG		display_max_children;
	char		*display_max_children_name;
	iniLONG     display_max_data;
	char		*display_max_data_name;
	iniLONG     display_max_depth;
	char		*display_max_depth_name;
	ytrace_var_runtime *var_export_runtime;
ZEND_END_MODULE_GLOBALS(ytrace)

/* In every utility function you add that needs to use variables 
   in php_ytrace_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as YTRACE_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define YTRACE_G(v) TSRMG(ytrace_globals_id, zend_ytrace_globals *, v)
#else
#define YTRACE_G(v) (ytrace_globals.v)
#endif

#endif	/* PHP_YTRACE_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
