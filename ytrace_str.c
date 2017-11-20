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

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "php.h"
#include "ext/standard/php_string.h"
#include "ytrace_str.h"

void ytrace_str_new(ytrace_str *ys)
{
	ys->c	= (char*)malloc(sizeof(char)*YTRACE_STR_START_SIZE);
	ys->c[0] = '\0';
	ys->len = 0;
	ys->a	= YTRACE_STR_START_SIZE;
}

void ytrace_str_append(ytrace_str *ys, char *str, int f)
{
	if (!str) return;
	int l = strlen(str);

	if (ys->len + l > ys->a - 1) {
		ys->c = (char*)realloc(ys->c, ys->a + l + YTRACE_STR_PREALLOC);
		ys->a = ys->a + l + YTRACE_STR_PREALLOC;
	}
	memcpy(ys->c + ys->len, str, l);
	ys->c[ys->len + l] = '\0';
	ys->len = ys->len + l;

	if (f) {
		free(str);
	}
}

void ytrace_str_appendl(ytrace_str *ys, char *str, int l, int f)
{
	if (!str) return;
	if (ys->len + l > ys->a - 1) {
		ys->c = (char*)realloc(ys->c, ys->a + l + YTRACE_STR_PREALLOC);
		ys->a = ys->a + l + YTRACE_STR_PREALLOC;
	}
	memcpy(ys->c + ys->len, str, l);
	ys->c[ys->len + l] = '\0';
	ys->len = ys->len + l;

	if (f) {
		free(str);
	}
}

void ytrace_str_chop(ytrace_str *ys, int c)
{
	if (c <= ys->len) {
		ys->len -= c;
		ys->c[ys->len] = '\0';
	}
}

void ytrace_str_destroy(ytrace_str *ys)
{
	if (ys->c) {
		free(ys->c);
	}
	ys->len = 0;
	ys->a = 0;
}

char* ytrace_str_addcslashes(char* str, int str_len, int *new_len)
{
#if PHP_VERSION_ID >= 70000
	zend_string *i_string = zend_string_init(str, str_len, 0);
	zend_string *tmp_zstr;

	tmp_zstr = php_addcslashes(i_string, 0, "'\\\0..\37", 6);

	char *tmp_str = estrndup(tmp_zstr->val, tmp_zstr->len);
	zend_string_release(tmp_zstr);
	zend_string_release(i_string);
	*new_len = tmp_zstr->len;
	return tmp_str;
#else
	return php_addcslashes(str, str_len, new_len, 0, "'\\\0..\37", 6 TSRMLS_CC);
#endif
}

char* ytrace_str_tab_pad(int len)
{
	int i;
	char *str = (char*)malloc(len);
	for (i = 0; i < len; i++) {
		str[i] = '\t';
	}
	return str;
}

// from https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
char** str_split(char* a_str, const char a_delim)
{
	a_str = strdup(a_str);
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

	free(a_str);

    return result;
}
