#ifndef ENV_HEADER
#define ENV_HEADER

#include <config.h>
#include <loader_types.h>

extern void env_init(loader_descriptor_p desc);
extern void env_print(void);
extern byte_t *env_get(byte_t *name);

#endif /* ENV_HEADER */
