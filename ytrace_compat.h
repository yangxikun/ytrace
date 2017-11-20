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

#ifndef __HAVE_YTRACE_COMPAT_H__
#define __HAVE_YTRACE_COMPAT_H__

#if PHP_VERSION_ID >= 70000
/* PHP 7 */

#define YTRACE_AUTO_GLOBAL(n) zend_is_auto_global_str(ZEND_STRL(n) TSRMLS_CC)

#define ZEND_USER_OPCODE_HANDLER_ARGS zend_execute_data *execute_data
#define YTRACE_APPLY_COUNT(ht) ZEND_HASH_GET_APPLY_COUNT(ht)
#define HASH_KEY_IS_NUMERIC(k) ((k) == NULL)
#define HASH_APPLY_KEY_VAL(k) (k)->val
#define HASH_APPLY_KEY_LEN(k) (k)->len + 1

#define STR_NAME_VAL(k) (char*)((k)->val)

#define iniLONG zend_long

#if PHP_VERSION_ID < 70100
/* object fetching in PHP 7.0
 *  * object = call ? Z_OBJ(call->This) : NULL; */
#define YTRACE_EX_OBJ(ex)   Z_OBJ(ex->This)
#else
/* object fetching in PHP 7.1
 *  * object = (call && (Z_TYPE(call->This) == IS_OBJECT)) ? Z_OBJ(call->This) : NULL; */
#define YTRACE_EX_OBJ(ex)   (Z_TYPE(ex->This) == IS_OBJECT ? Z_OBJ(ex->This) : NULL)
#endif
#define YTRACE_EX_OBJCE(ex) Z_OBJCE(ex->This)

#else
/* PHP 5 */

#define YTRACE_AUTO_GLOBAL(n) zend_is_auto_global(n, sizeof(n)-1 TSRMLS_CC)

#define ZEND_USER_OPCODE_HANDLER_ARGS ZEND_OPCODE_HANDLER_ARGS
#define YTRACE_APPLY_COUNT(ht) (ht->nApplyCount)
#define HASH_KEY_IS_NUMERIC(k) ((k)->nKeyLength == 0)
#define HASH_APPLY_NUMERIC(k) (k)->h
#define HASH_APPLY_KEY_VAL(k) (k)->arKey
#define HASH_APPLY_KEY_LEN(k) (k)->nKeyLength

#define STR_NAME_VAL(k) (char*)(k)

#define iniLONG long

#define YTRACE_EX_OBJ(ex)   ex->object
#define YTRACE_EX_OBJCE(ex) Z_OBJCE_P(ex->object)

#endif

#endif
