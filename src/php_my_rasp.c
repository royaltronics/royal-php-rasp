#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include <php_my_rasp.h>  // Custom header for our extension
#include <logger.h>       // Logger related functions and definitions
#include <overrides.h>    // Function override mechanisms

/*
 * This PHP extension provides a Runtime Application Self-Protection (RASP)
 * mechanism by overriding critical PHP functions to monitor and log their usage.
 * Functions that can be monitored include exec, system, popen, proc_open, and shell_exec.
 */

/* Declare any global variables; uncomment if needed.
ZEND_DECLARE_MODULE_GLOBALS(my_rasp)
*/

/* True global resources - not required to be thread-safe in this context */
static int le_my_rasp;

// Define the argument information for the confirm_my_rasp_compiled function
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_confirm_my_rasp_compiled, 0, 1, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, arg, IS_STRING, 0)
ZEND_END_ARG_INFO()

/*
 * Test function to confirm the extension is compiled and loaded.
 * {{{ proto string confirm_my_rasp_compiled(string arg)
 * This function returns a confirmation message along with the passed argument.
 */
PHP_FUNCTION(confirm_my_rasp_compiled) {
    char *arg = NULL;
    size_t arg_len;

    // Parse the passed argument from PHP function call
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
        return; // If parsing fails, return without executing
    }

    // Output the confirmation message with the passed argument
    php_printf("The 'my_rasp' extension is working and argument passed: %s\n", arg);
}
/* }}} */

/*
 * Module initialization function
 * {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(my_rasp) {
    // Define the list of functions to override
    static function_override overrides[] = {
        {"exec",   (void (**)(INTERNAL_FUNCTION_PARAMETERS))&original_exec_fptr,   PHP_FN(my_exec_override)},
        {"system", (void (**)(INTERNAL_FUNCTION_PARAMETERS))&original_system_fptr, PHP_FN(my_system_override)},
        {"popen",  (void (**)(INTERNAL_FUNCTION_PARAMETERS))&original_popen_fptr,  PHP_FN(my_popen_override)},
        {"proc_open", (void (**)(INTERNAL_FUNCTION_PARAMETERS))&original_proc_open_fptr, PHP_FN(my_proc_open_override)},
        {"shell_exec", (void (**)(INTERNAL_FUNCTION_PARAMETERS))&original_shell_exec_fptr, PHP_FN(my_shell_exec_override)},
        {"passthru", (void (**)(INTERNAL_FUNCTION_PARAMETERS))&original_passthru_fptr, PHP_FN(my_passthru_override)},
        //{"eval", (void (**)(INTERNAL_FUNCTION_PARAMETERS))&original_eval_fptr, PHP_FN(my_eval_override)},
        //{"assert", (void (**)(INTERNAL_FUNCTION_PARAMETERS))&original_assert_fptr, PHP_FN(my_assert_override)},
        // Additional functions can be added here
    };

    // Apply the function overrides
    override_functions(overrides, sizeof(overrides) / sizeof(overrides[0]));
    return SUCCESS;
}
/* }}} */

/* Function to restore original function pointers */
void restore_original_functions() {
    zend_function *func;

    // Restore original 'exec' function, if overridden
    if ((func = zend_hash_str_find_ptr(CG(function_table), "exec", strlen("exec"))) != NULL && original_exec_fptr != NULL) {
        func->internal_function.handler = original_exec_fptr;
    }

    // Restore original 'system' function, if overridden
    if ((func = zend_hash_str_find_ptr(CG(function_table), "system", strlen("system"))) != NULL && original_system_fptr != NULL) {
        func->internal_function.handler = original_system_fptr;
    }

    // Restore original 'popen' function, if overridden
    if ((func = zend_hash_str_find_ptr(CG(function_table), "popen", strlen("popen"))) != NULL && original_popen_fptr != NULL) {
        func->internal_function.handler = original_popen_fptr;
    }

    // Restore original 'proc_open' function, if overridden
    if ((func = zend_hash_str_find_ptr(CG(function_table), "proc_open", strlen("proc_open"))) != NULL && original_proc_open_fptr != NULL) {
        func->internal_function.handler = original_proc_open_fptr;
    }

    // Restore original 'shell_exec' function, if overridden
    if ((func = zend_hash_str_find_ptr(CG(function_table), "shell_exec", strlen("shell_exec"))) != NULL && original_shell_exec_fptr != NULL) {
        func->internal_function.handler = original_shell_exec_fptr;
    }

    // Restore original 'passthru' function, if overridden
    if ((func = zend_hash_str_find_ptr(CG(function_table), "passthru", strlen("passthru"))) != NULL && original_passthru_fptr != NULL) {
        func->internal_function.handler = original_passthru_fptr;
    }
}


/*
 * Module shutdown function
 * {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(my_rasp) {
    // Restore original function pointers if they were overridden
    restore_original_functions();
    return SUCCESS;
}
/* }}} */

/*
 * Module info function
 * {{{ PHP_MINFO_FUNCTION
 * Displays information about the module in PHP's info output
 */
PHP_MINFO_FUNCTION(my_rasp) {
    php_info_print_table_start();
    php_info_print_table_header(2, "my_rasp support", "enabled");
    php_info_print_table_end();
}
/* }}} */

/*
 * Module function entries
 * {{{ my_rasp_functions[]
 * List of functions provided by the extension
 */
const zend_function_entry my_rasp_functions[] = {
    PHP_FE(confirm_my_rasp_compiled, arginfo_confirm_my_rasp_compiled)
    PHP_FE_END  // Marks the end of function entries
};
/* }}} */

/*
 * Module entry point
 * {{{ my_rasp_module_entry
 */
zend_module_entry my_rasp_module_entry = {
    STANDARD_MODULE_HEADER,
    "my_rasp",           // Extension name
    my_rasp_functions,   // Functions provided by the extension
    PHP_MINIT(my_rasp),  // Module init function
    PHP_MSHUTDOWN(my_rasp), // Module shutdown function
    NULL,                // Request start function
    NULL,                // Request end function
    PHP_MINFO(my_rasp),  // Module info function
    PHP_MY_RASP_VERSION, // Extension version
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

/* Dynamic loading support */
#ifdef COMPILE_DL_MY_RASP
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(my_rasp)
#endif
