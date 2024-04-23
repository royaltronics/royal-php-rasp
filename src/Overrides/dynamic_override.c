 // dynamic_override.c
#include "php.h"
//#include <dynamic_override.h>
//#include <logger.h>

/* THIS FILE IS UNIMPLMENTED
 *
 *
 *
 */



 // Hashtable to store original function handlers
HashTable original_function_handlers;

// Register function overrides
void register_dynamic_overrides(void) {
    // Initialize the hashtable
    zend_hash_init(&original_function_handlers, 8, NULL, ZVAL_PTR_DTOR, 0);

    // Register our PHP functions to add and remove overrides
    REGISTER_STRING_CONSTANT("add_override", PHP_FN(add_override), CONST_CS | CONST_PERSISTENT);
    REGISTER_STRING_CONSTANT("remove_override", PHP_FN(remove_override), CONST_CS | CONST_PERSISTENT);
}

// Unregister function overrides and restore original functions
void unregister_dynamic_overrides(void) {
    // Iterate over the hashtable and restore original function handlers
    zend_string *key;
    zval *zv;
    ZEND_HASH_FOREACH_STR_KEY_VAL(&original_function_handlers, key, zv) {
        zend_function *original_func = (zend_function *)Z_PTR_P(zv);
        zend_hash_update_ptr(CG(function_table), key, original_func);
    } ZEND_HASH_FOREACH_END();

    // Destroy the hashtable
    zend_hash_destroy(&original_function_handlers);
}

// Generic override handler
PHP_FUNCTION(generic_override) {
    char *function_name = get_active_function_name();
    zval *args = NULL;
    int num_args = ZEND_NUM_ARGS();

    // Allocate space for the arguments, if any
    if (num_args > 0) {
        args = (zval *)safe_emalloc(num_args, sizeof(zval), 0);
        if (zend_get_parameters_array_ex(num_args, args) == FAILURE) {
            efree(args);
            WRONG_PARAM_COUNT;
        }
    }

    // Log function execution details
    char log_details[1024];
    snprintf(log_details, sizeof(log_details), "Function executed: %s", function_name);
    log_call(EG(current_execute_data), function_name, log_details);

    // Retrieve original function pointer from a global store (you need to implement this part)
    zend_function *original_func = get_original_func(function_name);

    // If original function pointer was stored, call the original function
    if (original_func != NULL) {
        // Prepare to call the original function
        zval retval;
        original_func->internal_function.handler(num_args, args, &retval, NULL, 1 TSRMLS_CC);

        // Return value to the caller
        if (!EG(exception)) {
            if (Z_TYPE(retval) != IS_UNDEF) {
                RETVAL_ZVAL(&retval, 1, 1);
            } else {
                RETVAL_NULL();
            }
        }

        // Cleanup
        if (args) {
            efree(args);
        }
        zval_ptr_dtor(&retval);
    } else {
        php_error_docref(NULL, E_WARNING, "Original function for %s not found", function_name);
        RETVAL_FALSE;
    }
}

// Note: Implement get_original_func() to retrieve the original function pointer
// based on the function name. This should match your storage mechanism where
// you keep the original function pointers when overriding them.


// PHP function to add a new override
PHP_FUNCTION(add_override) {
    char *function_name;
    size_t function_name_len;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STRING(function_name, function_name_len)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    // Check if function already exists
    zend_function *existing_func = zend_hash_str_find_ptr(CG(function_table), function_name, function_name_len);
    if (!existing_func) {
        php_error_docref(NULL, E_WARNING, "Function %s does not exist", function_name);
        RETURN_FALSE;
    }

    // Store original function handler
    zend_hash_str_add_ptr(&original_function_handlers, function_name, function_name_len, existing_func);

    // Override function handler with new one
    existing_func->internal_function.handler = fcc.function_handler;

    RETURN_TRUE;
}

// PHP function to remove an override
PHP_FUNCTION(remove_override) {
    char *function_name;
    size_t function_name_len;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STRING(function_name, function_name_len)
    ZEND_PARSE_PARAMETERS_END();

    // Find original function handler
    zend_function *original_func = zend_hash_str_find_ptr(&original_function_handlers, function_name, function_name_len);
    if (!original_func) {
        php_error_docref(NULL, E_WARNING, "No override found for function %s", function_name);
        RETURN_FALSE;
    }

    // Restore original function handler
    zend_hash_update_ptr(CG(function_table), zend_string_init(function_name, function_name_len, 0), original_func);

    // Remove from our hashtable
    zend_hash_str_del(&original_function_handlers, function_name, function_name_len);

    RETURN_TRUE;
}
