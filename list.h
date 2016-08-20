#ifndef __LIST_H
#define __LIST_H

typedef struct List_tag *List;
List list_create(void(*data_free)(void *data),
	void(*data_print)(void *data));
void list_free(List list);
int list_get_number(List list);
int list_insert_at_index(List list, int index, void *data);
void *list_get_index(List list, int index);
void *list_pop_index(List list, int index);
void list_remove_index(List list, int index);
void list_delete_index(List list, int index);
void list_print(List list);
#endif