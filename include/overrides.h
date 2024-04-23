#ifndef OVERRIDES_H
#define OVERRIDES_H

#include "php.h"
#include "logger.h"

// Global variables to store the original function pointers
static void (*original_exec_fptr)(INTERNAL_FUNCTION_PARAMETERS) = NULL;
static void (*original_system_fptr)(INTERNAL_FUNCTION_PARAMETERS) = NULL;
static void (*original_popen_fptr)(INTERNAL_FUNCTION_PARAMETERS) = NULL;
static void (*original_proc_open_fptr)(INTERNAL_FUNCTION_PARAMETERS) = NULL;
static void (*original_shell_exec_fptr)(INTERNAL_FUNCTION_PARAMETERS) = NULL;
static void (*original_passthru_fptr)(INTERNAL_FUNCTION_PARAMETERS) = NULL;
//static void (*original_eval_fptr)(INTERNAL_FUNCTION_PARAMETERS) = NULL;
//static void (*original_assert_fptr)(INTERNAL_FUNCTION_PARAMETERS) = NULL;

/**
 * passthru
 * eval
 * assert
 */


/**
 * A structure to facilitate overriding PHP functions.
 * It contains the name of the function to override, a pointer to store the original function,
 * and a new function pointer for the overriding behavior.
 */
typedef struct _function_override {
    const char *name; // Name of the function to override.
    void (**original_fptr)(INTERNAL_FUNCTION_PARAMETERS); // Storage for the original function pointer.
    void (*override_fptr)(INTERNAL_FUNCTION_PARAMETERS); // The new, overriding function pointer.
} function_override;

/**
 * Applies an array of function overrides to the PHP runtime.
 * Each override specified in the array will replace a PHP internal function
 * with a custom implementation, while preserving the original function pointer.
 *
 * @param overrides An array of function_override structures defining which functions to override.
 * @param count Number of elements in the overrides array.
 */
void override_functions(const function_override overrides[], size_t count) {
    for (size_t i = 0; i < count; ++i) {
        zend_function *original_function = zend_hash_str_find_ptr(CG(function_table), overrides[i].name, strlen(overrides[i].name));
        if (original_function != NULL) {
            *(overrides[i].original_fptr) = original_function->internal_function.handler;
            original_function->internal_function.handler = overrides[i].override_fptr;
        }
    }
}


/* ********************************************* */
/*                                               */
/*            Blacklisted Commands               */
/*                                               */
/* ********************************************* */

/*
 * The list below specifies commands that are considered dangerous and are
 * therefore blacklisted from being used within the PHP environment.
 *
 * It's possible to implement an additional feature that performs a hash
 * check on the caller function to identify legitimate uses of these
 * blacklisted commands. This would involve maintaining a 'whitelist' of
 * approved function signatures. If a function calling a blacklisted command
 * matches a signature on this whitelist, the command could be allowed to
 * execute.
 *
 * This approach allows for flexibility and ensures that essential system
 * functionalities are not inadvertently blocked while still maintaining
 * robust security against potential script injections or unauthorized
 * command executions.
 */

const char* blacklisted_commands[] = {
    "whoami",
    "ls",
    "rm",
    "dd",
    "mkfs",
    "fdisk",
    "wget",
    "curl",
    "ssh",
    "scp",
    "iptables",
    "ufw",
    "nc",
    "telnet",
    "python",
    "perl",
    "ruby",
    "php",
    "gcc",
    "make",
    "sudo",
    "su",
    "crontab",
    "chown",
    "chmod",
    "setuid",
    "setgid",
    "rsync",
    "git",
    "mount",
    "umount",
    "passwd",
    NULL  // Null terminate the array
};

/* Function to check if command contains any blacklisted command */
bool is_command_blacklisted(const char *command) {
    if (command == NULL) return false; // Safety check

    for (int i = 0; blacklisted_commands[i] != NULL; i++) {
        if (strstr(command, blacklisted_commands[i]) != NULL) {
            // Found a blacklisted command within the command string
            return true;
        }
    }

    // No blacklisted commands found in the command string
    return false;
}



// Function overrides follow here. Each should:
// 1. Parse the parameters.
// 2. Log the call details.
// 3. Call the original function (if applicable).

// Each override function should perform similar steps: parse parameters, log call, call original

/**
 * Overrides for PHP's exec function.
 * Logs the function call details and then calls the original exec function.
 */
PHP_FUNCTION(my_exec_override) {
    char *command = NULL;
    size_t command_len;
    zval *output = NULL;
    zval *return_var = NULL;

    // Initialize the output array if needed
    zval output_array;
    int needs_output_cleanup = 0;

    // Parse the incoming arguments
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|a!z!", &command, &command_len, &output, &return_var) == FAILURE) {
        RETURN_FALSE; // Argument parsing failed.
    }


    // If 'output' is not provided, create a temporary array for it
    if (!output) {
        array_init(&output_array);
        output = &output_array;
        needs_output_cleanup = 1; // Mark for cleanup later
    }

    // Log command execution details
    char log_details[1024];
    snprintf(log_details, sizeof(log_details), "Command executed: %s", command);
    if (is_command_blacklisted(command)) {
        //snprintf(log_details, sizeof(log_details), "BLOCKED: %s", command);
        //php_error_docref(NULL, E_WARNING, "Attempt to execute blacklisted command: %s", command);
        log_call(EG(current_execute_data), "exec", log_details, true);

        RETURN_FALSE; // Prevent the execution of the blacklisted command

    }


    log_call(EG(current_execute_data), "exec", log_details, false);

    // Call the original 'exec' function with the arguments passed to this override
    if (original_exec_fptr != NULL) {
        original_exec_fptr(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    }

    // Cleanup if a temporary output array was used
    if (needs_output_cleanup) {
        zval_ptr_dtor(output); // Only clean up if we initialized it
    }
}


/**
 * Overrides for PHP's system function.
 * Logs the function call details and then calls the original system function.
 */
PHP_FUNCTION(my_system_override) {
    char *command = NULL;
    size_t command_len;
    zval *return_var = NULL;  // This will capture the return status if provided.

    // Parse the incoming arguments. 's' for string, '|z' for an optional zval.
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|z!", &command, &command_len ,&return_var) == FAILURE) {
        RETURN_FALSE;  // Argument parsing failed.
    }

    // Log command execution details.
    char log_details[1024];
    snprintf(log_details, sizeof(log_details), "System command executed: %s", command);
    if (is_command_blacklisted(command)) {
        //snprintf(log_details, sizeof(log_details), "BLOCKED: %s", command);
        //php_error_docref(NULL, E_WARNING, "Attempt to execute blacklisted command: %s", command);
        log_call(EG(current_execute_data), "system", log_details, true);

        RETURN_FALSE; // Prevent the execution of the blacklisted command

    }


    log_call(EG(current_execute_data), "system", log_details, false);

    // Call the original 'system' function with the arguments passed to this override.
    // Ensure that the original function pointer exists before trying to call it.
    if (original_system_fptr != NULL) {
        // Pass all arguments as they were received. This uses the INTERNAL_FUNCTION_PARAM_PASSTHRU macro.
        original_system_fptr(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    } else {
        // Log an error if the original function pointer is not set.
        php_error_docref(NULL, E_WARNING, "Original 'system' function not found.");
        RETURN_FALSE;
    }
}


/**
 * Overrides for PHP's popen function.
 * Logs the function call details and then calls the original popen function.
 */
PHP_FUNCTION(my_popen_override) {
    char *command = NULL, *mode = NULL;
    size_t command_len, mode_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", &command, &command_len, &mode, &mode_len) == FAILURE) {
        RETURN_FALSE; // Incorrect parameters, return false
    }

    if (!command || !mode) {
        php_error_docref(NULL, E_WARNING, "Missing command or mode parameter.");
        RETURN_FALSE;
    }

    // Log command execution details
    char log_details[1024];
    snprintf(log_details, sizeof(log_details), "popen command executed: %s, mode: %s", command, mode);
    if (is_command_blacklisted(command)) {
        //snprintf(log_details, sizeof(log_details), "BLOCKED: %s", command);
        //php_error_docref(NULL, E_WARNING, "Attempt to execute blacklisted command: %s", command);
        log_call(EG(current_execute_data), "popen", log_details, true);

        RETURN_FALSE; // Prevent the execution of the blacklisted command

    }


    log_call(EG(current_execute_data), "popen", log_details, false);

    // If original popen function pointer is valid, call it and return its result
    if (original_popen_fptr != NULL) {
        original_popen_fptr(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    } else {
        php_error_docref(NULL, E_WARNING, "Original 'popen' function not found.");
        RETURN_FALSE;
    }
}


/**
 * Overrides for PHP's proc_open function.
 * Logs the function call details and then calls the original proc_open function.
 */
PHP_FUNCTION(my_proc_open_override) {
    zval *command;
    zval *descriptorspec;
    zval *pipes = NULL;  // Initialize pipes as NULL.
    char *cwd = NULL;
    size_t cwd_len = 0;
    zval *env = NULL;
    zval *other_options = NULL;

    // Prepare a temporary array for pipes if needed.
    zval temp_pipes;
    array_init(&temp_pipes);

    //Have fixed the zend_parse_parameters to allow NULLABLE pipes.
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "zaa!/|s!a!a!", &command, &descriptorspec, &pipes, &cwd, &cwd_len, &env, &other_options) == FAILURE) {
        zval_dtor(&temp_pipes); // Clean up the temporary array if parameters parsing failed.
        RETURN_FALSE;
    }

    // If 'pipes' is not provided (or NULL), use the temporary array.
    if (!pipes) {
        pipes = &temp_pipes;
    } else {
        ZVAL_DEREF(pipes); // Dereference it to ensure it's not a reference to NULL.
        if (Z_TYPE_P(pipes) != IS_ARRAY) {
            // Convert it to an array if it's not already an array.
            convert_to_array(pipes);
        }
    }

    // Log command execution details...
    char log_details[1024];
    if (Z_TYPE_P(command) == IS_STRING) {
        snprintf(log_details, sizeof(log_details), "proc_open command executed: %s, CWD: %s", Z_STRVAL_P(command), cwd ? cwd : "N/A");
    } else if (Z_TYPE_P(command) == IS_ARRAY) {
        snprintf(log_details, sizeof(log_details), "proc_open command executed: [Array Command], CWD: %s", cwd ? cwd : "N/A");
    } else {
        snprintf(log_details, sizeof(log_details), "proc_open command executed: [Unknown Command Type], CWD: %s", cwd ? cwd : "N/A");
    }

    if (is_command_blacklisted(command)) {
        //snprintf(log_details, sizeof(log_details), "BLOCKED: %s", command);
        //php_error_docref(NULL, E_WARNING, "Attempt to execute blacklisted command: %s", command);
        log_call(EG(current_execute_data), "proc_open", log_details, true);

        RETURN_FALSE; // Prevent the execution of the blacklisted command

    }

    log_call(EG(current_execute_data), "proc_open", log_details, false);

    // Call the original 'proc_open' function with the arguments passed to this override.
    if (original_proc_open_fptr != NULL) {
        original_proc_open_fptr(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    } else {
        php_error_docref(NULL, E_WARNING, "Original 'proc_open' function not found.");
        RETURN_FALSE;
    }

    // Clean up the temporary pipes array if it was used.
    if (pipes == &temp_pipes) {
        zval_dtor(&temp_pipes);
    }

    //RETURN_TRUE; // Signify success.
}



/**
 * Overrides for PHP's shell_exec function.
 * Logs the function call details and then calls the original shell_exec function.
 */
PHP_FUNCTION(my_shell_exec_override) {
    char *command = NULL;
    size_t command_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &command, &command_len) == FAILURE) {
        RETURN_FALSE; // Argument parsing failed.
    }

    char log_details[1024];
    snprintf(log_details, sizeof(log_details), "shell_exec command executed: %s", command);

    if (is_command_blacklisted(command)) {
        //snprintf(log_details, sizeof(log_details), "BLOCKED: %s", command);
        //php_error_docref(NULL, E_WARNING, "Attempt to execute blacklisted command: %s", command);
        log_call(EG(current_execute_data), "shell_exec", log_details, true);

        RETURN_FALSE; // Prevent the execution of the blacklisted command

    }

    log_call(EG(current_execute_data), "shell_exec", log_details, true);

    if (original_shell_exec_fptr != NULL) {
        original_shell_exec_fptr(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    }

    //RETURN_TRUE; // No return value for shell_exec, so just signify success
}


/**
 * Overrides for PHP's passthru function.
 * Logs the function call details and then calls the original passthru function.
 */
PHP_FUNCTION(my_passthru_override) {
    char *command = NULL;
    size_t command_len;
    zval *return_var = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|z!", &command, &command_len, &return_var) == FAILURE) {
        RETURN_FALSE;  // Argument parsing failed.
    }

    char log_details[1024];
    snprintf(log_details, sizeof(log_details), "passthru command executed: %s", command);
    log_call(EG(current_execute_data), "passthru", log_details, false);

    if (original_passthru_fptr != NULL) {
        original_passthru_fptr(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    }
}

/**
 * Overrides for PHP's eval function.
 * Logs the function call details and then calls the original eval function.
 *
 * non-functional
 */
/*
PHP_FUNCTION(my_eval_override) {
    char *code = NULL;
    size_t code_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &code, &code_len) == FAILURE) {
        RETURN_FALSE;  // Argument parsing failed.
    }

    char log_details[1024];
    snprintf(log_details, sizeof(log_details), "eval code executed: %s", code);
    log_call(EG(current_execute_data), "eval", log_details, false);

    if (original_eval_fptr != NULL) {
        original_eval_fptr(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    }
}
*/
/**
 * Overrides for PHP's assert function.
 * Logs the function call details and then calls the original assert function.
 *
 * non-functional
 */

/*
PHP_FUNCTION(my_assert_override) {
    zval *assertion;
    zval *description = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|z!", &assertion, &description) == FAILURE) {
        RETURN_FALSE;  // Argument parsing failed.
    }

    char *assert_str = NULL;
    if (Z_TYPE_P(assertion) == IS_STRING) {
        assert_str = Z_STRVAL_P(assertion);
    } else {
        assert_str = "Non-string assertion";
    }

    char log_details[1024];
    if (description && Z_TYPE_P(description) == IS_STRING) {
        snprintf(log_details, sizeof(log_details), "assert executed: %s, description: %s", assert_str, Z_STRVAL_P(description));
    } else {
        snprintf(log_details, sizeof(log_details), "assert executed: %s", assert_str);
    }
    log_call(EG(current_execute_data), "assert", log_details, false);

    if (original_assert_fptr != NULL) {
        original_assert_fptr(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    }
}

*/

#endif /* OVERRIDES_H */
