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

#ifndef __HAVE_YTRACE_STR_H__
#define __HAVE_YTRACE_STR_H__

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>

#define	YTRACE_STR_START_SIZE	78
#define	YTRACE_STR_PREALLOC		128

typedef struct {
	char *c;
	size_t len;
	size_t a;
} ytrace_str;

void ytrace_str_new(ytrace_str* ys);
void ytrace_str_append(ytrace_str *ys, char *str, int free);
void ytrace_str_appendl(ytrace_str *ys, char *str, int l, int free);
void ytrace_str_chop(ytrace_str *xs, int c);
void ytrace_str_destroy(ytrace_str *str);

char* ytrace_str_addcslashes(char* str, int str_len, int *new_len);
char* ytrace_str_tab_pad(int len);
char** str_split(char* a_str, const char a_delim);

/* Set correct int format to use */
#if PHP_VERSION_ID >= 70000
# include "Zend/zend_long.h"
# if SIZEOF_ZEND_LONG == 4
#  define YTRACE_INT_FMT "%ld"
# else
#  define YTRACE_INT_FMT "%lld"
# endif
#else
# define YTRACE_INT_FMT "%ld"
#endif

static char *ytrace_sprintf(const char *fmt, ...)
{
   int size = 0;
   char *p = NULL;
   va_list ap;

   /* Determine required size */

   va_start(ap, fmt);
   size = vsnprintf(p, size, fmt, ap);
   va_end(ap);

   if (size < 0)
	   return NULL;

   size++;             /* For '\0' */
   p = malloc(size);
   if (p == NULL)
	   return NULL;

   va_start(ap, fmt);
   size = vsnprintf(p, size, fmt, ap);
   if (size < 0) {
	   free(p);
	   return NULL;
   }
   va_end(ap);

   return p;
}

/* __HAVE_YTRACE_STR_H__ */
#endif
