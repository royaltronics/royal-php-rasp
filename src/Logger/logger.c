#include "php.h"
#include "SAPI.h"
#include "ext/standard/info.h"
#include <sha256.h>

// Helper function to read and compute SHA256 hash of a file
char* sha256_hash_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) return NULL;

    SHA256_CTX ctx;
    sha256_init(&ctx);

    const int bufSize = 1024; // Read 1024 bytes at a time
    uint8_t *buffer = (uint8_t*)malloc(bufSize);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    int bytesRead;
    while ((bytesRead = fread(buffer, 1, bufSize, file)) > 0) {
        sha256_update(&ctx, buffer, bytesRead);
    }

    uint8_t hash[32];
    sha256_final(&ctx, hash);

    fclose(file);
    free(buffer);

    // Convert the hash to a hexadecimal string
    char *hashStr = (char*)malloc(65); // SHA-256 strings are 64 characters plus a null terminator
    if (!hashStr) return NULL;

    for (int i = 0; i < 32; i++) {
        sprintf(hashStr + (i * 2), "%02x", hash[i]);
    }
    hashStr[64] = 0; // Null terminator

    return hashStr; // The caller is responsible for freeing this memory
}

/**
 * Escapes special characters in a string to ensure it is valid JSON.
 * This function is used to prepare strings to be embedded in JSON structures
 * safely by escaping characters such as quotes, backslashes, and control characters.
 *
 * @param str The string to be escaped.
 * @return A newly allocated string with escaped characters, which must be freed by the caller.
 */
char* escape_json_string(const char* str) {
    if (str == NULL) return NULL; // Ensure the input string is not NULL

    size_t len = strlen(str);
    // Calculate the additional space required for escaping special characters.
    size_t additional_space = 0;
    for (size_t i = 0; i < len; ++i) {
        switch (str[i]) {
            // One extra character needed for each escape sequence
            case '\"': case '\\': case '\b':
            case '\f': case '\n': case '\r':
            case '\t':
                additional_space++;
                break;
            default:
                break; // No additional space needed for other characters
        }
    }

    // Allocate space for the new string, including space for escape characters and null terminator.
    char* escaped_str = (char*)malloc(len + additional_space + 1);
    if (!escaped_str) return NULL; // Return NULL if memory allocation failed

    // Construct the escaped string
    size_t j = 0; // Index for filling the new string
    for (size_t i = 0; i < len; ++i) {
        // Replace special characters with their escaped counterparts
        switch (str[i]) {
            case '\"': escaped_str[j++] = '\\'; escaped_str[j++] = '\"'; break;
            case '\\': escaped_str[j++] = '\\'; escaped_str[j++] = '\\'; break;
            case '\b': escaped_str[j++] = '\\'; escaped_str[j++] = 'b';  break;
            case '\f': escaped_str[j++] = '\\'; escaped_str[j++] = 'f';  break;
            case '\n': escaped_str[j++] = '\\'; escaped_str[j++] = 'n';  break;
            case '\r': escaped_str[j++] = '\\'; escaped_str[j++] = 'r';  break;
            case '\t': escaped_str[j++] = '\\'; escaped_str[j++] = 't';  break;
            default:   escaped_str[j++] = str[i]; break; // Copy other characters directly
        }
    }
    escaped_str[j] = '\0'; // Ensure the string is null-terminated

    return escaped_str; // Return the escaped string
}

/**
 * Safe function to extract string from php array
 *
 * todo: documentation
 **/
char *extract_string_from_array(zval *array, const char *key) {
    zval *value = zend_hash_str_find(Z_ARRVAL_P(array), key, strlen(key));
    if (value && Z_TYPE_P(value) == IS_STRING) {
        return Z_STRVAL_P(value);
    }
    return "unknown";
}

/**
 * Sanitizes a PHP filename string to remove specifics added by eval() and other dynamic PHP executions.
 * This function removes file details added after eval() and trims line numbers from the filename.
 *
 * @param original_filename The original filename as extracted from Zend execute data.
 * @param line_number The linenumber of the eval line if eval is found
 * @param is_eval Represents if eval is found in filename for log_call
 * @return A new string with sanitized filename, which should be freed by the caller.
 */
char* sanitize_php_filename(const char *original_filename, uint *line_number, bool *is_eval) {
    if (original_filename == NULL) {
        return strdup("unknown");
    }

    char *sanitized_filename = strdup(original_filename);
    if (!sanitized_filename) {
        return strdup("unknown");
    }

    *is_eval = false;  // Preset is_eval as false


    char *eval_part = strstr(sanitized_filename, " : eval()'d code");
    if (eval_part) {
        *is_eval = true;  // Set is_eval to true because we found eval part

    if (is_eval)
        *line_number = 0;  // Reset line number as default

        // Walk backwards to find the first '(' before " : eval()'d code"
        char *parenthesis = NULL;
        for (char *p = eval_part; p > sanitized_filename; --p) {
            if (*p == ')') {
                *p = '\0';  // Null-terminate at the closing parenthesis
                parenthesis = p;  // Mark the position of the closing parenthesis
            } else if (*p == '(' && parenthesis) {
                *line_number = atoi(p + 1); // Extract the line number
                *p = '\0'; // Null-terminate at the opening parenthesis
                break;
            }
        }
    }

    return sanitized_filename;
}




/**
 * Logs a call to a PHP function, storing the details in a JSON-formatted log file.
 * This function is used to log potentially vulnerable or monitored function calls,
 * including their arguments and the context in which they were called.
 *
 * @param exec_data Execution data from the Zend engine providing context for the call.
 * @param type The type of log entry (e.g., "WARNING", "ERROR").
 * @param details Detailed information about the log entry.
 * @param was_blocked Represents if the call was blocked or not.
 */
void log_call(zend_execute_data *exec_data, const char *type, const char *details, bool was_blocked) {
    // Attempt to open the log file for appending
    FILE *file = fopen("/var/www/logfile.log", "a");
    //PLEASE READ: PHP-CLI Runs as local user, MAKE SURE CLI user has access to this log directory, as well as http user.
    if (file == NULL) {
        // Log an error using PHP's error handling if the file cannot be opened
        php_error_docref(NULL, E_WARNING, "Unable to open log file for writing.");
        return;
    }

    // Retrieve the current timestamp
    time_t now = time(NULL);
    char time_str[30]; // Buffer to store formatted timestamp
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

    // Attempt to fetch the REMOTE_ADDR from SAPI
    char *client_ip = NULL;
    if (sapi_module.getenv) {
        client_ip = sapi_module.getenv("REMOTE_ADDR", strlen("REMOTE_ADDR"));
    }

    // Prepare and escape the log entry components to ensure they are safe for JSON
    char *escaped_type = escape_json_string(type);
    char *escaped_details = escape_json_string(details);
    char *escaped_function_name = "unknown";
    char *escaped_filename = "unknown";
    char *escaped_ip = escape_json_string(client_ip);
    char *file_hash = NULL;
    char *escaped_last_modified_time;
    char *escaped_file_hash = NULL;
    uint lineno = 0;

    bool is_eval = 0;


    // Extract function call details if available
    if (exec_data && exec_data->func) {
        zend_execute_data *caller = exec_data->prev_execute_data ? exec_data->prev_execute_data : exec_data;
        const char *raw_function_name = caller->func->common.function_name ? ZSTR_VAL(caller->func->common.function_name) : "unknown";
        const char *raw_filename = caller->func->op_array.filename ? ZSTR_VAL(caller->func->op_array.filename) : "unknown";
        lineno = caller->opline ? caller->opline->lineno : 0;

        // Escape the extracted strings
        escaped_function_name = escape_json_string(raw_function_name);
        int line_number;



        char *sanitized_filename = sanitize_php_filename(raw_filename, &lineno, &is_eval); // Use the new function here.
        escaped_filename = escape_json_string(sanitized_filename);

        if (sanitized_filename) {
            file_hash = sha256_hash_file(sanitized_filename);
            escaped_file_hash = escape_json_string(file_hash);

            char last_modified_time[30] = "unknown";
            struct stat attr;
            if (sanitized_filename && stat(sanitized_filename, &attr) == 0) {
                strftime(last_modified_time, sizeof(last_modified_time), "%Y-%m-%d %H:%M:%S", localtime(&(attr.st_mtime)));
            }

            if (last_modified_time) {
                escaped_last_modified_time = escape_json_string(last_modified_time);
            }
        }
    }

    // Write the log entry in JSON format to the file
    fprintf(file, "{\"timestamp\":\"%s\", \"type\":\"%s\", \"details\":\"%s\", \"caller\":\"%s\", \"filename\":\"%s\", \"file-hash\":\"%s\", \"modified-time\":\"%s\", \"line\":%u, \"ip\":\"%s\", \"is-eval\":%s, \"was-blocked\":%s}\n",
            time_str,
            escaped_type ? escaped_type : "unknown",
            escaped_details ? escaped_details : "unknown",
            escaped_function_name ? escaped_function_name : "unknown",
            escaped_filename ? escaped_filename : "unknown",
            escaped_file_hash,
            escaped_last_modified_time,
            lineno, escaped_ip,
            is_eval ? "true" : "false",
            was_blocked ? "true" : "false");

    // Free allocated resources
    if (escaped_type) free(escaped_type);
    if (escaped_details) free(escaped_details);
    if (escaped_function_name && escaped_function_name != "unknown") free(escaped_function_name);
    if (escaped_filename && escaped_filename != "unknown") free(escaped_filename);
    if (escaped_ip) free(escaped_ip);
    if (file_hash) free(file_hash);
    if (escaped_file_hash) free(escaped_file_hash);
    if (escaped_last_modified_time) free(escaped_last_modified_time);

    // Close the log file
    fclose(file);
}
