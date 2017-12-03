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

#include "php.h"
#include "php_ytrace.h"
#include "ext/standard/php_lcg.h"
#include "ytrace_str.h"

ZEND_EXTERN_MODULE_GLOBALS(ytrace)

#define MICRO_IN_SEC 1000000.00

double ytrace_get_utime(void)
{
#ifdef HAVE_GETTIMEOFDAY
	struct timeval tp;
	long sec = 0L;
	double msec = 0.0;

	if (gettimeofday((struct timeval *) &tp, NULL) == 0) {
		sec = tp.tv_sec;
		msec = (double) (tp.tv_usec / MICRO_IN_SEC);

		if (msec >= 1.0) {
			msec -= (long) msec;
		}
		return msec + sec;
	}
#endif
	return 0;
}

void ytrace_format_output_filename(ytrace_str *filename, char *format)
{
	char       cwd[128];
	TSRMLS_FETCH();

	ytrace_str_append(filename, "/", 0);
	while (*format)
	{
		if (*format != '%') {
			ytrace_str_appendl(filename, (char *) format, 1, 0);
		} else {
			format++;
			switch (*format)
			{
				case 'p': /* pid */
					ytrace_str_append(filename, ytrace_sprintf("%ld", getpid()), 1);
					break;

				case 'r': /* random number */
					ytrace_str_append(filename, ytrace_sprintf("%06x", (long) (1000000 * php_combined_lcg(TSRMLS_C))), 1);
					break;

				case 't': { /* timestamp (in seconds) */
					time_t the_time = time(NULL);
					ytrace_str_append(filename, ytrace_sprintf("%ld", the_time), 1);
				}	break;

				case 'u': { /* timestamp (in microseconds) */
					char *char_ptr, *utime_str = ytrace_sprintf("%F", ytrace_get_utime());

					/* Replace . with _ (or should it be nuked?) */
					char_ptr = strrchr(utime_str, '.');
					if (char_ptr) {
						char_ptr[0] = '_';
					}
					ytrace_str_append(filename, utime_str, 1);
				}	break;

				case 'H':   /* $_SERVER['HTTP_HOST'] */
				case 'U':   /* $_SERVER['UNIQUE_ID'] */
				case 'R': { /* $_SERVER['REQUEST_URI'] */
					char *char_ptr, *strval;
#if PHP_VERSION_ID >= 70000
					zval *data = NULL;

					if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) == IS_ARRAY) {
						switch (*format) {
						case 'H':
							data = zend_hash_str_find(Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]), "HTTP_HOST", sizeof("HTTP_HOST") - 1);
							break;
						case 'R':
							data = zend_hash_str_find(Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]), "REQUEST_URI", sizeof("REQUEST_URI") - 1);
							break;
						case 'U':
							data = zend_hash_str_find(Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]), "UNIQUE_ID", sizeof("UNIQUE_ID") - 1);
							break;
						}

						if (data) {
							strval = estrdup(Z_STRVAL_P(data));
#else
					int retval = FAILURE;
					zval **data;

					if (PG(http_globals)[TRACK_VARS_SERVER]) {
						switch (*format) {
						case 'H':
							retval = zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), "HTTP_HOST", sizeof("HTTP_HOST"), (void **) &data);
							break;
						case 'R':
							retval = zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), "REQUEST_URI", sizeof("REQUEST_URI"), (void **) &data);
							break;
						case 'U':
							retval = zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), "UNIQUE_ID", sizeof("UNIQUE_ID"), (void **) &data);
							break;
						}

						if (retval == SUCCESS) {
							strval = estrdup(Z_STRVAL_PP(data));
#endif

							char *pos = strstr(strval, "?");
							if (pos) {
								*pos = '\0';
							}
							/* replace slashes, dots, question marks, plus
							 * signs, ampersands, spaces and other evil chars
							 * with underscores */
							while ((char_ptr = strpbrk(strval, "/\\.?&+:*\"<>| ")) != NULL) {
								char_ptr[0] = '_';
							}
							ytrace_str_append(filename, strval, 0);
							efree(strval);
						}
					}
				}	break;

				case '%': /* literal % */
					ytrace_str_appendl(filename, "%", 1, 0);
					break;
			}
		}
		format++;
	}
	ytrace_str_append(filename, ".yt", 0);
}
