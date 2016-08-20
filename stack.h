#ifndef __STACK_H
#define __STACK_H

typedef struct Stack_tag *Stack;
Stack stack_create(void(*data_free)(void *data),
	void(*data_print)(void *data));
void stack_push(Stack stack, void *data);
void *stack_pop(Stack stack);
void *stack_get_top(Stack stack);
void stack_delete_top(Stack stack);
int stack_is_empty(Stack stack);
void stack_free(Stack stack);
void stack_print(Stack stack);
#endif