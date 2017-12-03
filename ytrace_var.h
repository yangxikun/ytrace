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

#ifndef __HAVE_YTRACE_VAR_H__
#define __HAVE_YTRACE_VAR_H__

#include "zend_compile.h"
#include "ytrace_str.h"

typedef struct
{
	int current;
} ytrace_var_runtime;

zval *ytrace_get_zval(zend_execute_data *zdata, int node_type, const znode_op *node);
void ytrace_get_zval_value(zval *val, ytrace_str *str);
void ytrace_var_export(zval **struc, ytrace_str *str, int level TSRMLS_DC);
char* ytrace_get_val_from_server(char *var_name);
char* ytrace_get_val_from_http_data(char *var_name);

/* __HAVE_YTRACE_VAR_H__ */
#endif
