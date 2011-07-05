#include "env.h"
#include "string.h"
#include "heap.h"

#include <loader.h>
#include <drivers/console_iface.h>

#define ENV_MAX_SIZE	0xFF*2

typedef struct envvar_s {
	byte_t *name;
	byte_t *value;
	void *next;
} envvar_t, *envvar_p;

static envvar_p envlist;

#define ENV_SEP			'='
#define ENV_MSK			'\\'

static envvar_p env_alloc_variable(void) {
	envvar_p res = 0;
	if (res = malloc(sizeof(envvar_t))) {
		res->name = 0;
		res->value = 0;
		res->next = 0;
	}
	return res;
}

static void env_free_variable(envvar_p ptr) {
	if(ptr->name)	free(ptr->name);
	if(ptr->value)	free(ptr->value);
	free(ptr);
}

void env_parse(byte_t *env) {

	byte_t buffer[ENV_MAX_SIZE];
	byte_t *ptr = buffer;
	byte_t *dst = 0;
	envvar_p var = envlist;
	byte_t c;
	byte_t msk = 0;
	while (c = *env++) {

		switch(c) {
		case '\n':
			*ptr = 0;
			dst = malloc(strlen(buffer)+1);
			if (dst) strcpy(dst,buffer);
			if (!var->value && dst) var->value = dst;
			ptr = buffer;
			dst = 0;
			var->next = env_alloc_variable();
			var = var->next;
			break;
		case ENV_MSK:
			msk = 1;
			break;
		case ENV_SEP:
			if (!msk) {
				*ptr = 0;
				dst = malloc(strlen(buffer)+1);
				if (dst) strcpy(dst,buffer);
				if (!var->name && dst) var->name = dst;
				ptr = buffer;
				dst = 0;
				break;
			}
		default:
			*ptr++ = c;
			msk = 0;
		}
	}
	*ptr = 0;
	dst = malloc(strlen(buffer)+1);
	if (dst) strcpy(dst,buffer);
	if (!var->value && dst) var->value = dst;
}

void env_init(loader_descriptor_p desc) {
	envlist = env_alloc_variable();
	env_parse(LOADER_ENV(desc));
}

byte_t *env_get(byte_t *name) {
	envvar_p var = envlist;
	while (var) {
		if (!strcmp(name,var->name)) {
			return var->value;
		}
		var = var->next;
	}
	return 0;
}

void env_set(byte_t *name, byte_t *value) {
	envvar_p var = envlist;
	envvar_p prev = 0;
	while (var) {
		if (!strcmp(name,var->name)) {
			break;
		}
		prev = var;
		var = var->next;
	}
	if (!var) {
		var = env_alloc_variable();
		prev->next = var;
		var->name = malloc(strlen(name)+1);
		if (var->name) strcpy(var->name,name);
	}
	var->value = malloc(strlen(value)+1);
	if (var->value) strcpy(var->value,value);
}

void env_print(void) {
	envvar_p var = envlist;
	printf("================ Environment variables ================\n\r");
	while (var) {
		if (var->name)
			printf("%s:", var->name);
		if (var->value)
			printf("\t%s", var->value);
		printf("\n\r");
		var = var->next;
	}
	printf("=======================================================\n\r");
}
