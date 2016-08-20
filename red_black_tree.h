#ifndef __RED_BLACK_TREE_H
#define __RED_BLACK_TREE_H


typedef struct Red_Black_Tree_tag *RB_Tree;
typedef struct Red_Black_Tree_Iterator_tag *RB_Tree_Iterator;
typedef struct Red_Black_Tree_Node_Pair_tag *RB_Tree_Node_Pair;
// red black tree create
RB_Tree red_black_tree_create(int(*key_compare)(void* k1, void* k2),
	void(*key_free)(void* k),
	void(*value_free)(void* v),
	void(*key_print)(void* k),
	void(*value_print)(void *v));
void red_black_tree_free(RB_Tree tree);
void red_black_tree_insert(RB_Tree, void *k, void *v);
void *red_black_tree_search(RB_Tree tree, void *key);
void *red_black_tree_pop(RB_Tree tree, void *k);
void red_black_tree_remove(RB_Tree tree, void *k);
void red_black_tree_delete(RB_Tree tree, void *k);
void red_black_tree_print(RB_Tree tree);
int red_black_tree_get_number(RB_Tree tree);
RB_Tree_Iterator red_black_tree_iterator_create(RB_Tree tree);
void red_black_tree_iterator_free(RB_Tree_Iterator iterator);
RB_Tree_Node_Pair red_black_tree_iterator_get_next(RB_Tree_Iterator iterator);
void *red_black_tree_node_pair_get_key(RB_Tree_Node_Pair pair);
void *red_black_tree_node_pair_get_value(RB_Tree_Node_Pair pair);

typedef RB_Tree Map;
typedef RB_Tree_Iterator Map_Iterator;
typedef RB_Tree_Node_Pair Map_Node_Pair;
Map map_create(int(*key_compare)(void* k1, void* k2),
	void(*key_free)(void* k),
	void(*value_free)(void* v),
	void(*key_print)(void* k),
	void(*value_print)(void *v));
void map_free(Map m);
void map_insert(Map, void *k, void *v);
void *map_search(Map m, void *key);
void *map_pop(Map m, void *k);
void map_remove(Map m, void *k);
void map_delete(Map m, void *k);
void map_print(Map m);
int map_get_number(Map m);
Map_Iterator map_iterator_create(Map m);
void map_iterator_free(Map_Iterator mi);
Map_Node_Pair map_iterator_get_next(Map_Iterator mi);
void *map_node_pair_get_key(Map_Node_Pair pair);
void *map_node_pair_get_value(Map_Node_Pair pair);
#endif
