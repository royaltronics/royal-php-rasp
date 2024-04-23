#ifndef LOGGER_H
#define LOGGER_H

/* Including necessary PHP and Zend engine headers for logging functionality */
#include "php.h"
//#include "zend_types.h"

/**
 * Function prototype for log_call.
 * This function is designed to log calls to potentially vulnerable PHP functions.
 * It generates a JSON-formatted log entry which includes details such as the type of event,
 * specific details provided, and context from the Zend execution environment.
 *
 * @param exec_data Execution data from the Zend engine providing context for the call.
 * @param type A string indicating the type of log entry (e.g., "ERROR", "WARNING").
 * @param details A string providing detailed information about the event being logged.
 */
void log_call(zend_execute_data *exec_data, const char *type, const char *details, bool was_blocked);

#endif /* LOGGER_H */
