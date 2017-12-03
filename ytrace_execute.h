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

#ifndef __HAVE_YTRACE_EXECUTE_H__
#define __HAVE_YTRACE_EXECUTE_H__

#include "Zend/zend_API.h"

#if PHP_VERSION_ID >= 70000

void (*ytrace_old_execute_ex)(zend_execute_data *execute_data TSRMLS_DC);
void ytrace_execute_ex(zend_execute_data *execute_data TSRMLS_DC);

void (*ytrace_old_execute_internal)(zend_execute_data *current_execute_data, zval *return_value);
void ytrace_execute_internal(zend_execute_data *current_execute_data, zval *return_value);

#else

void (*ytrace_old_execute_ex)(zend_execute_data *execute_data TSRMLS_DC);
void ytrace_execute_ex(zend_execute_data *execute_data TSRMLS_DC);

void (*ytrace_old_execute_internal)(zend_execute_data *current_execute_data, zend_fcall_info *fci, int return_value_used TSRMLS_DC);
void ytrace_execute_internal(zend_execute_data *current_execute_data, zend_fcall_info *fci, int return_value_used TSRMLS_DC);

#endif

/* __HAVE_YTRACE_EXECUTE_H__ */
#endif
