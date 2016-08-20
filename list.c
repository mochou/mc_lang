#include <stdlib.h>
#include <stdio.h>
#include "list.h"
#include "LOG.h"
typedef struct Node_tag *Node;

struct Node_tag
{
	void *data;
	struct Node_tag *prev;
	struct Node_tag *next;
};

struct List_tag
{
	int number; // the number of elements
	Node head;
	void(*data_free)(void *data);
	void(*data_print)(void *data);
};

// Node
Node _node_create(void *data);
void _node_free(Node node, void(*data_free)(void *data));

// List 

void _list_insert_at_index(List list, int index, Node node);
Node _list_get_index(List list, int index);
void _list_delete_index(List list, int index);

// implements
static Node _node_create(void *data)
{
	Node node = malloc(sizeof(*node));
	if (node == NULL)
	{
		LOG_ERROR("out of memory");
		LOG_EXIT();
	}
	node->data = data;
	return node;
}
static void _node_free_data(Node node, void(*data_free)(void *data))
{
	if (node == NULL)
	{
		return;
	}
	if (node->data != NULL)
	{
		data_free(node->data);
	}
}
static void _node_free_self(Node node)
{
	if (node == NULL)
	{
		return;
	}
	free(node);
}
static void _node_free(Node node, void(*data_free)(void *data))
{
	if (node == NULL)
	{
		return;
	}
	_node_free_data(node, data_free);
	_node_free_self(node);
}

/***********************************
***********TOOL FUNCTION************
***********************************/
static Node _list_get_index(List list, int index)
{
	int i = 0;
	Node iterator = list->head->next;
	while (i < index)
	{
		i++;
		iterator = iterator->next;
	}
	return iterator;
}

static void _list_remove_index(List list, int index)
{
	Node node = _list_get_index(list, index);
	Node prev = node->prev;
	Node next = node->next;
	prev->next = next;
	if (next != NULL)
	{
		next->prev = prev;
	}
	list->number--;
}
static void _list_delete_index(List list, int index)
{
	Node node = _list_get_index(list, index);
	_list_remove_index(list, index);
	_node_free(node, list->data_free);
}

static void _list_insert_at_index(List list, int index, Node node)
{
	int i = 0;
	Node iterator = list->head;
	while (i < index)
	{
		i++;
		iterator = iterator->next;
	}
	node->prev = iterator;
	node->next = iterator->next;
	iterator->next = node;
	if (node->next != NULL)
	{
		node->next->prev = node;
	}
	list->number++;
}
/***********************************
*********INTERFACE FUNCTION*********
***********************************/
List list_create(void(*data_free)(void *data),
	void(*data_print)(void *data))
{
	List list = malloc(sizeof(*list));
	if (list == NULL)
	{
		LOG_INFO("out of memory");
		exit(-1);
	}
	if (data_free == NULL)
	{
		LOG_INFO("function data_free is NULL");
	}
	if (data_print == NULL)
	{
		LOG_INFO("function data_print is NULL");
	}
	list->data_free = data_free;
	list->data_print = data_print;
	
	list->number = 0;
	Node head = _node_create(NULL);
	head->prev = head->next = NULL;
	list->head = head;
	return list;
}

void list_free(List list)
{
	if (list == NULL)
	{
		LOG_INFO("list is NULL");
		exit(-1);
	}
	while (list->number > 0)
	{
		list_delete_index(list, 0);
	}
}

int list_insert_at_index(List list, int index, void *data)
{
	if (list == NULL)
	{
		LOG_INFO("list is NULL");
		exit(-1);
	}
	if (index > list->number)
	{
		LOG_INFO("invalid index");
		return 1;
	}	
	Node node = _node_create(data);
	_list_insert_at_index(list, index, node);
	return 0;
}

void *list_get_index(List list, int index)
{
	if (list == NULL)
	{
		LOG_INFO("list is NULL");
		exit(-1);
	}
	if (index >= list->number)
	{
		LOG_INFO("invalid index");
		return NULL;
	}
	Node node = _list_get_index(list, index);
	if (node == NULL)
	{
		return NULL;
	}
	else
	{
		return node->data;
	}
}

void *list_pop_index(List list, int index)
{
	if (list == NULL)
	{
		LOG_INFO("list is NULL");
		exit(-1);
	}
	if (index >= list->number)
	{
		LOG_INFO("invalid index");
		return NULL;
	}
	void *data = NULL;
	Node node = _list_get_index(list, index);
	if (node == NULL)
	{
		return NULL;
	}
	else
	{
		data = node->data;
	}
	_list_remove_index(list, index);
	_node_free_self(node);
	return data;
}

void list_remove_index(List list, int index)
{
	if (list == NULL)
	{
		LOG_INFO("list is NULL");
		exit(-1);
	}
	if (index >= list->number)
	{
		LOG_INFO("invalid index\n");
		return;
	}
	Node node = _list_get_index(list, index);
	if (node == NULL)
	{
		return;
	}
	_list_remove_index(list, index);	
	_node_free_self(node);
}

void list_delete_index(List list, int index)
{
	if (list == NULL)
	{
		LOG_INFO("list is NULL");
		exit(-1);
	}
	if (index >= list->number)
	{
		LOG_INFO("invalid index\n");
		return;
	}
	_list_delete_index(list, index);
}

void list_print(List list)
{
	if (list == NULL)
	{
		LOG_INFO("list is NULL");
		exit(-1);
	}
	void *data = NULL;
	int index = 0;
	for (index = 0; index < list->number; index++)
	{
		data = list_get_index(list, index);
		list->data_print(data);
		if (index + 1 < list->number)
		{
			LOG_INFO(", ");
		}
	}
}

int list_get_number(List list)
{
	if (list == NULL)
	{
		LOG_INFO("list is NULL");
		exit(-1);
	}
	return list->number;
}