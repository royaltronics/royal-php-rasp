// dynamic_override.h
#ifndef DYNAMIC_OVERRIDE_H
#define DYNAMIC_OVERRIDE_H

#include "php.h"

// Function prototypes
PHP_FUNCTION(add_override);
PHP_FUNCTION(remove_override);
PHP_FUNCTION(generic_override)
void register_dynamic_overrides(void);
void unregister_dynamic_overrides(void);

#endif /* DYNAMIC_OVERRIDE_H */
