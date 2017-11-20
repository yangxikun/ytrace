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

#include <string.h>
#include "php.h"
#include "php_ytrace.h"
#include "ytrace_str.h"

ZEND_EXTERN_MODULE_GLOBALS(ytrace)

int ytrace_should_trace(char *filename)
{
	char **patterns;
	int  i;
	int  match = 0;

	if (strlen(YTRACE_G(white_list)) > 0) {
		patterns = str_split(YTRACE_G(white_list), ',');
		if (patterns) {
			for (i = 0; *(patterns + i); i++)
			{
				if (!match && strstr(filename, *(patterns + i)) != NULL) {
					match = 1;
				}
				free(*(patterns + i));
			}
			free(patterns);
		}
		return match;
	}

	if (strlen(YTRACE_G(black_list)) > 0) {
		patterns = str_split(YTRACE_G(black_list), ',');
		if (patterns) {
			for (i = 0; *(patterns + i); i++)
			{
				if (!match && strstr(filename, *(patterns + i)) != NULL) {
					match = 1;
				}
				free(*(patterns + i));
			}
			free(patterns);
		}
		return !match;
	}

	return 1;
}
