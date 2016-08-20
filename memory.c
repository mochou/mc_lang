#include "environment.h"
#include <stdio.h>
#include <stdlib.h>
#include "LOG.h"

void *MEM_malloc(int size)
{
	void *ptr = malloc(size);
	if (ptr == NULL)
	{
		LOG_ERROR("out of memory");
		LOG_EXIT();
	}
	return ptr;
}

void MEM_free(void * ptr)
{
	free(ptr);
}

void *MEM_realloc(void *ptr, int new_size)
{
	void *new_ptr = NULL;
	new_ptr = realloc(ptr, new_size);
	if (new_ptr == NULL)
	{
		LOG_ERROR("out of memory");
		LOG_EXIT();
	}
	return new_ptr;
}