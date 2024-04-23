#ifndef PHP_MY_RASP_H
#define PHP_MY_RASP_H

extern zend_module_entry my_rasp_module_entry;
#define phpext_my_rasp_ptr &my_rasp_module_entry

#define PHP_MY_RASP_VERSION "0.3.0"

#ifdef PHP_WIN32
#   define PHP_MY_RASP_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define PHP_MY_RASP_API __attribute__ ((visibility("default")))
#else
#   define PHP_MY_RASP_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_FUNCTION(confirm_my_rasp_compiled);

/*
   Declare any global variables you may need between the BEGIN
   and END macros here:

ZEND_BEGIN_MODULE_GLOBALS(my_rasp)
    long  global_value;
    char *global_string;
ZEND_END_MODULE_GLOBALS(my_rasp)
*/

/* Always refer to the globals in your function as MY_RASP_G(rm) */

#endif /* PHP_MY_RASP_H */
