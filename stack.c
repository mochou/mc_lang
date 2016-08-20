#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"
#include "list.h"
#include "LOG.h"

struct Stack_tag
{
	List list;
};

Stack stack_create(void(*data_free)(void *data),
	void(*data_print)(void *data))
{
	Stack stack = malloc(sizeof(*stack));	
	stack->list = list_create(data_free, data_print);
	return stack;
}

void stack_push(Stack stack, void *data)
{
	list_insert_at_index(stack->list, 0, data);
}

void *stack_pop(Stack stack)
{
	if (stack == NULL)
	{
		LOG_INFO("stack is NULL");
		exit(-1);
	}
	void *data = NULL;
	if (list_get_number(stack->list) > 0)
	{
		data = list_pop_index(stack->list, 0);
		return data;
	}
	else
	{
		return NULL;
	}
}

void *stack_get_top(Stack stack)
{
	if (stack == NULL)
	{
		LOG_INFO("stack is NULL");
		exit(-1);
	}
	void *data = NULL;
	if (list_get_number(stack->list) > 0)
	{
		data = list_get_index(stack->list, 0);
		return data;
	}
	else
	{
		return NULL;
	}
}
void stack_delete_top(Stack stack)
{
	if (stack == NULL)
	{
		LOG_INFO("stack is NULL");
		exit(-1);
	}
	if (list_get_number(stack->list) > 0)
	{
		list_delete_index(stack->list, 0);
	}
}

int stack_is_empty(Stack stack)
{
	if (stack == NULL)
	{
		LOG_INFO("stack is NULL");
		exit(-1);
	}
	return list_get_number(stack->list) == 0;
}

void stack_free(Stack stack)
{
	if (stack == NULL)
	{
		LOG_INFO("stack is NULL");
		exit(-1);
	}
	list_free(stack->list);
	free(stack);
}

void stack_print(Stack stack)
{
	list_print(stack->list);
}