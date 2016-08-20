#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "red_black_tree.h"
#include "LOG.h"

#define RED_BLACK_TREE_RED 1
#define RED_BLACK_TREE_BLACK 0
//#define __RED_BLACK_TREE_PRINT_COLOR
#define __RED_BLACK_TREE_PRINT_KEY
#define __RED_BLACK_TREE_PRINT_VALUE
//#define __RED_BLACK_TREE_PRINT_PARENT
//#define __RED_BLACK_TREE_PRINT_CHILDREN


/*********************************************************************/
typedef struct Red_Black_Node_tag *RB_Node;

struct Red_Black_Node_tag
{
	int id;
	int color;
	void *key;
	void *value;
	RB_Node left;
	RB_Node right;
	RB_Node parent;
};

struct Red_Black_Tree_tag
{
	int number;
	RB_Node root;
	RB_Node nil;
	// aux function;
	int (*key_compare)(void* k1, void* k2);
	void (*key_free)(void* k);
	void (*value_free)(void* v);
	void (*key_print)(void* k);
	void (*value_print)(void *v);
};

struct Red_Black_Tree_Node_Pair_tag
{
	void *key;
	void *value;
};

struct Red_Black_Tree_Iterator_tag
{
	RB_Tree tree;
	int next_index;
	int number;
	RB_Tree_Node_Pair *list;
};
// node
RB_Node _red_black_node_create(void *k, void *v);
void _red_black_node_free(RB_Node node,
	void(*key_free)(void *k),
	void(*value_free)(void *v));

// tree
// insert
void _red_black_tree_left_rotate(RB_Tree tree, RB_Node node);
void _red_black_tree_right_rotate(RB_Tree tree, RB_Node node);
void _red_black_tree_insert(RB_Tree tree, RB_Node node);
void _red_black_tree_insert_fixup(RB_Tree tree, RB_Node node);
// delete
void _red_black_tree_transplant(RB_Tree tree, RB_Node u, RB_Node v);
void _red_black_tree_delete(RB_Tree tree, RB_Node node);
void _red_black_tree_remove(RB_Tree tree, RB_Node node);
void _red_black_tree_remove_fixup(RB_Tree tree, RB_Node node);
// free
void _red_black_tree_free(RB_Tree tree, RB_Node node);
// tree search
RB_Node _red_black_tree_search(RB_Tree tree, void *key);
// tree print
 void _red_black_tree_print(RB_Tree tree, RB_Node node);
// 
 RB_Node _red_black_subtree_maximum(RB_Tree tree, RB_Node subtree);
 RB_Node _red_black_subtree_minimum(RB_Tree tree, RB_Node subtree);

// implements
// node
 static RB_Node _red_black_node_create(void *k, void *v)
{
	RB_Node node = MEM_malloc(sizeof(*node));
	node->key = k;
	node->value = v;

	node->color = -1;
	node->id = -1;
	node->parent = node->left = node->right = NULL;
	return node;
}

 static void _red_black_node_free_key(RB_Node node,
	 void(*key_free)(void *k))
 {
	 if (node == NULL)
	 {
		 return;
	 }
	 if (node->key != NULL)
	 {
		 key_free(node->key);
		 node->key = NULL;
	 }
 }
 static void _red_black_node_free_value(RB_Node node,
	 void(*value_free)(void *v))
 {
	 if (node == NULL)
	 {
		 return;
	 }	 
	 if (node->value != NULL)
	 {
		 value_free(node->value);
		 node->value = NULL;
	 }
 }

 static void _red_black_node_free_self(RB_Node node)
 {
	 if (node == NULL)
	 {
		 return;
	 }
	 MEM_free(node);
 }
 static void _red_black_node_free(RB_Node node,
	void (*key_free)(void *k), 
	void (*value_free)(void *v))
{
	if (node == NULL)
	{
		return;
	}
	_red_black_node_free_key(node, key_free);
	_red_black_node_free_value(node, value_free);
	_red_black_node_free_self(node);
}

// tree
/****************************************
*************TOOL FUNCTION***************
****************************************/
 static void _red_black_tree_left_rotate(RB_Tree tree, RB_Node x)
 {
	 RB_Node y = x->right;
	 x->right = y->left;
	 if (y->left != tree->nil)
	 {
		 y->left->parent = x;
	 }
	 y->parent = x->parent;
	 if (x->parent == tree->nil)
	 {
		 tree->root = y;
	 }
	 else if (x == x->parent->left)
	 {
		 x->parent->left = y;
	 }
	 else if (x == x->parent->right)
	 {
		 x->parent->right = y;
	 }
	 y->left = x;
	 x->parent = y;
 }
 static void _red_black_tree_right_rotate(RB_Tree tree, RB_Node x)
 {
	 RB_Node y = x->left;
	 x->left = y->right;
	 if (y->right != tree->nil)
	 {
		 y->right->parent = x;
	 }
	 y->parent = x->parent;
	 if (x->parent == tree->nil)
	 {
		 tree->root = y;
	 }
	 else if (x == x->parent->left)
	 {
		 x->parent->left = y;
	 }
	 else if (x == x->parent->right)
	 {
		 x->parent->right = y;
	 }
	 y->right = x;
	 x->parent = y;
 }
 static void _red_black_tree_insert(RB_Tree tree, RB_Node z)
 {
	 z->id = tree->number++;
	 RB_Node y = tree->nil;
	 RB_Node x = tree->root;
	 while (x != tree->nil)
	 {
		 y = x;
		 if (tree->key_compare(z->key, x->key) < 0)
		 {
			 x = x->left;
		 }
		 else
		 {
			 x = x->right;
		 }
	 }
	 z->parent = y;
	 if (y == tree->nil)
	 {
		 tree->root = z;
	 }
	 else if (tree->key_compare(z->key, y->key) < 0)
	 {
		 y->left = z;
	 }
	 else
	 {
		 y->right = z;
	 }
	 z->left = tree->nil;
	 z->right = tree->nil;
	 z->color = RED_BLACK_TREE_RED;
	 _red_black_tree_insert_fixup(tree, z);
 }

 static void _red_black_tree_insert_fixup(RB_Tree tree, RB_Node z)
 {
	 RB_Node y = NULL;
	 while (z->parent->color == RED_BLACK_TREE_RED)
	 {
		 if (z->parent == z->parent->parent->left)
		 {
			 /* z's parent is the left child of z's parent's parent*/
			 y = z->parent->parent->right;

			 if (y->color == RED_BLACK_TREE_RED)
			 {
				 //case 1: y.color is RED
				 z->parent->color = RED_BLACK_TREE_BLACK;
				 y->color = RED_BLACK_TREE_BLACK;
				 z->parent->parent->color = RED_BLACK_TREE_RED;
				 z = z->parent->parent;
			 }
			 else
			 {
				 // the color of z's uncle is BLACK
				 // case 2
				 if (z == z->parent->right)
				 {
					 z = z->parent;
					 _red_black_tree_left_rotate(tree, z);
				 }
				 // case 3
				 z->parent->color = RED_BLACK_TREE_BLACK;
				 z->parent->parent->color = RED_BLACK_TREE_RED;
				 _red_black_tree_right_rotate(tree, z->parent->parent);
			 }
		 }
		 else if (z->parent == z->parent->parent->right)
		 {
			 /* z's parent is the right child of z's parent's parent*/
			 y = z->parent->parent->left;
			 //case 1
			 if (y->color == RED_BLACK_TREE_RED)
			 {
				 z->parent->color = RED_BLACK_TREE_BLACK;
				 y->color = RED_BLACK_TREE_BLACK;
				 z->parent->parent->color = RED_BLACK_TREE_RED;
				 z = z->parent->parent;
			 }
			 else
			 {
				 // case 2
				 if (z == z->parent->left)
				 {
					 z = z->parent;
					 _red_black_tree_right_rotate(tree, z);
				 }
				 // go to case 3
				 z->parent->color = RED_BLACK_TREE_BLACK;
				 z->parent->parent->color = RED_BLACK_TREE_RED;
				 _red_black_tree_left_rotate(tree, z->parent->parent);
			 }
		 }

	 }
	 tree->root->color = RED_BLACK_TREE_BLACK;
 }

 static RB_Node _red_black_subtree_maximum(RB_Tree tree, RB_Node subtree)
 {
	 while (subtree->right != tree->nil)
	 {
		 subtree = subtree->right;
	 }
	 return subtree;
 }

 static RB_Node _red_black_subtree_minimum(RB_Tree tree, RB_Node subtree)
 {
	 while (subtree->left != tree->nil)
	 {
		 subtree = subtree->left;
	 }
	 return subtree;
 }

 static void _red_black_tree_transplant(RB_Tree tree, RB_Node u, RB_Node v)
 {
	 if (u->parent == tree->nil)
	 {
		 tree->root = v;
	 }
	 else if (u == u->parent->left)
	 {
		 u->parent->left = v;
	 }
	 else
	 {
		 u->parent->right = v;
	 }
	 v->parent = u->parent;
 }

 static void _red_black_tree_free(RB_Tree tree, RB_Node node)
 {
	 if (node != tree->nil)
	 {
		 _red_black_tree_free(tree, node->left);
		 _red_black_tree_free(tree, node->right);
		 _red_black_node_free(node, tree->key_free, tree->value_free);
		 node = NULL;
	 }
 }
 static void _red_black_tree_remove_fixup(RB_Tree tree, RB_Node x)
 {
	 RB_Node w = NULL;
	 while (x != tree->root && x->color == RED_BLACK_TREE_BLACK)
	 {
		 if (x == x->parent->left)
		 {
			 // left child
			 w = x->parent->right;
			 if (w->color == RED_BLACK_TREE_RED)
			 {
				 // case 1
				 w->color = RED_BLACK_TREE_BLACK;
				 x->parent->color = RED_BLACK_TREE_RED;
				 _red_black_tree_left_rotate(tree, x->parent);
				 w = x->parent->right;
			 }
			 // go to case 2, 3 or 4
			 if (w->left->color == RED_BLACK_TREE_BLACK && w->right->color == RED_BLACK_TREE_BLACK)
			 {
				 // case 2
				 w->color = RED_BLACK_TREE_RED;
				 x = x->parent;
			 }
			 else
			 {
				 // case 3 or 4
				 if (w->right->color == RED_BLACK_TREE_BLACK)
				 {
					 // case 3
					 w->left->color = RED_BLACK_TREE_BLACK;
					 w->color = RED_BLACK_TREE_RED;
					 _red_black_tree_right_rotate(tree, w);
					 w = x->parent->right;
				 }
				 w->color = x->parent->color;
				 x->parent->color = RED_BLACK_TREE_BLACK;
				 w->right->color = RED_BLACK_TREE_BLACK;
				 _red_black_tree_left_rotate(tree, x->parent);
				 x = tree->root;
			 }
		 }
		 else if (x == x->parent->right)
		 {
			 // right child
			 w = x->parent->left;
			 if (w->color == RED_BLACK_TREE_RED)
			 {
				 // case 1
				 w->color = RED_BLACK_TREE_BLACK;
				 x->parent->color = RED_BLACK_TREE_RED;
				 _red_black_tree_right_rotate(tree, x->parent);
				 w = x->parent->left;
			 }
			 // go to case 2, 3 or 4
			 if (w->right->color == RED_BLACK_TREE_BLACK && w->left->color == RED_BLACK_TREE_BLACK)
			 {
				 // case 2
				 w->color = RED_BLACK_TREE_RED;
				 x = x->parent;
			 }
			 else
			 {
				 // case 3 or 4
				 if (w->left->color == RED_BLACK_TREE_BLACK)
				 {
					 // case 3
					 w->right->color = RED_BLACK_TREE_BLACK;
					 w->color = RED_BLACK_TREE_RED;
					 _red_black_tree_left_rotate(tree, w);
					 w = x->parent->right;
				 }
				 w->color = x->parent->color;
				 x->parent->color = RED_BLACK_TREE_BLACK;
				 w->left->color = RED_BLACK_TREE_BLACK;
				 _red_black_tree_right_rotate(tree, x->parent);
				 x = tree->root;
			 }
		 }
	 }
	 x->color = RED_BLACK_TREE_BLACK;
 }
 static void _red_black_tree_remove(RB_Tree tree, RB_Node z)
 {
	 tree->number--;
	 RB_Node y = z;
	 RB_Node x = NULL;
	 int y_original_color = y->color;

	 if (z->left == tree->nil)
	 {
		 x = z->right;
		 _red_black_tree_transplant(tree, z, z->right);
	 }
	 else if (z->right == tree->nil)
	 {
		 x = z->left;
		 _red_black_tree_transplant(tree, z, z->left);
	 }
	 else
	 {
		 y = _red_black_subtree_minimum(tree, z->right);
		 y_original_color = y->color;
		 x = y->right;

		 if (y->parent == z)
		 {
			 x->parent = y;
		 }
		 else
		 {
			 _red_black_tree_transplant(tree, y, y->right);
			 y->right = z->right;
			 y->right->parent = y;
		 }
		 _red_black_tree_transplant(tree, z, y);
		 y->left = z->left;
		 y->left->parent = y;
		 y->color = z->color;
	 }

	 if (y_original_color == RED_BLACK_TREE_BLACK)
	 {
		 _red_black_tree_remove_fixup(tree, x);
	 }

	 /*********DO NOTHING********/
	 // NO NEED to PROCESS the node z
}
 
 static void _red_black_tree_delete(RB_Tree tree, RB_Node z)
 {
	 _red_black_tree_remove(tree, z);
	 _red_black_node_free(z, tree->key_free, tree->value_free);
 }
 
 static RB_Node _red_black_tree_search(RB_Tree tree, void *key)
 {
	 RB_Node node = tree->root;
	 while (node != tree->nil && tree->key_compare(key, node->key) != 0)
	 {
		 //if (key < node->key)
		 if (tree->key_compare(key, node->key) < 0)
		 {
			 node = node->left;
		 }
		 else
		 {
			 node = node->right;
		 }
	 }
	 if (node == tree->nil)
	 {
		 return NULL;
	 }
	 return node;
 }
 static void _red_black_tree_print(RB_Tree tree, RB_Node node)
 {
	 if (node != tree->nil)
	 {
		 _red_black_tree_print(tree, node->left);
#ifdef __RED_BLACK_TREE_PRINT_KEY
		 LOG_INFO("key: ");
		 tree->key_print(node->key);
		 LOG_INFO(" ");
#endif
#ifdef __RED_BLACK_TREE_PRINT_VALUE
		 LOG_INFO("value: ");
		 tree->value_print(node->value);
		 LOG_INFO(" ");
#endif
#ifdef __RED_BLACK_TREE_PRINT_COLOR
		 LOG_INFO("%s ", node->color == RED_BLACK_TREE_BLACK ? "B" : "R");
#endif
#ifdef __RED_BLACK_TREE_PRINT_PARENT
		 LOG_INFO("parent: ");
		 if (node->parent->key != NULL)
		 {
			 tree->key_print(node->parent->key);
		 }
		 else
		 {
			 LOG_INFO("NIL");
		 }
		 LOG_INFO(" ");
#endif
#ifdef __RED_BLACK_TREE_PRINT_CHILDREN
		 LOG_INFO("left: ");
		 if (node->left->key != NULL)
		 {
			 tree->key_print(node->left->key);
		 }
		 else
		 {
			 LOG_INFO("NIL");
		 }
		 LOG_INFO(" ");
		 LOG_INFO("right: ");
		 if (node->right->key != NULL)
		 {
			 tree->key_print(node->right->key);
		 }
		 else
		 {
			 LOG_INFO("NIL");
		 }
#endif
		 //LOG_INFO(" parent: %d, left: %d, right: %d}", node->parent->id, node->left->id, node->right->id);
		 LOG_INFO("\n");
		 _red_black_tree_print(tree, node->right);
	 }
 }
/************************************
***********INTERFACE FUNCTION********
*************************************/
RB_Tree red_black_tree_create(int(*key_compare)(void* k1, void* k2),
	void(*key_free)(void* k),
	void(*value_free)(void* v),
	void(*key_print)(void* k),
	void(*value_print)(void *v))
{
	RB_Tree tree = (RB_Tree)MEM_malloc(sizeof(*tree));
	// initialize function
	if (key_compare == NULL)
	{
		LOG_INFO("function key_compare not initialized");
		exit(-1);
	}
	if (key_free == NULL)
	{
		LOG_INFO("function key_free not initialized");
		exit(-1);
	}
	if (value_free == NULL)
	{
		LOG_INFO("function value_free not initialized");
		exit(-1);
	}
	if (key_print == NULL)
	{
		LOG_INFO("function key_print not initialized");
		exit(-1);
	}
	if (value_print == NULL)
	{
		LOG_INFO("function value_print not initialized");
		exit(-1);
	}

	tree->key_compare = key_compare;
	tree->key_free = key_free;
	tree->value_free = value_free;
	tree->key_print = key_print;
	tree->value_print = value_print;

	tree->nil = _red_black_node_create(NULL, NULL);
	tree->nil->id = 0;
	tree->nil->color = RED_BLACK_TREE_BLACK;
	tree->nil->key = NULL;
	tree->nil->value = NULL;
	tree->nil->left = NULL;
	tree->nil->right = NULL;
	tree->nil->parent = NULL;
	tree->root = tree->nil;
	tree->number = 0;
	return tree;
}

void red_black_tree_insert(RB_Tree tree, void *k, void *v)
{
	if (tree == NULL)
	{
		return;
	}
	RB_Node node = _red_black_node_create(k, v);
	_red_black_tree_insert(tree, node);	
}

void red_black_tree_free(RB_Tree tree)
{
	if (tree == NULL)
	{
		return;
	}
	_red_black_tree_free(tree, tree->root);
	_red_black_node_free(tree->nil, tree->key_free, tree->value_free);
	MEM_free(tree);
}

void *red_black_tree_search(RB_Tree tree, void *key)
{
	if (tree == NULL)
	{
		return NULL;
	}
	RB_Node node = _red_black_tree_search(tree, key);
	if (node == NULL)
	{
		return NULL;
	}
	else
	{
		return node->value;
	}
}

void *red_black_tree_pop(RB_Tree tree, void *key)
{
	if (tree == NULL)
	{
		return NULL;
	}
	void *value = NULL;
	RB_Node node = _red_black_tree_search(tree, key);
	if (node == NULL)
	{
		return NULL;
	}
	else
	{
		value = node->value;
	}
	_red_black_tree_remove(tree, node);
	_red_black_node_free_key(node, tree->key_free);
	_red_black_node_free_self(node);
	return value;
}

void red_black_tree_remove(RB_Tree tree, void *k)
{
	if (tree == NULL)
	{
		return;
	}
	RB_Node node = _red_black_tree_search(tree, k);
	if (node == NULL)
	{
		return;
	}
	_red_black_tree_remove(tree, node);
	_red_black_node_free_key(node, tree->key_free);
	_red_black_node_free_self(node);
}

void red_black_tree_delete(RB_Tree tree, void *k)
{
	if (tree == NULL)
	{
		return;
	}
	RB_Node node = _red_black_tree_search(tree, k);
	if (node == NULL)
	{
		return;
	}
	_red_black_tree_delete(tree, node);
}

void red_black_tree_print(RB_Tree tree)
{
	if (tree == NULL)
	{
		return;
	}
	_red_black_tree_print(tree, tree->root);
	//LOG_INFO("\n");
}

int red_black_tree_get_number(RB_Tree tree)
{
	return tree->number;
}
static void _red_black_tree_iterator_created_by_inorder_traversal(RB_Tree tree, RB_Node node, 
	int *next_index, void **list, int number)
{
	if (node == tree->nil)
	{
		return;
	}
	_red_black_tree_iterator_created_by_inorder_traversal(tree, node->left, next_index, list, number);
	if (*next_index >= number)
	{
		LOG_ERROR("next index out of number");
		LOG_EXIT();
	}
	RB_Tree_Node_Pair node_pair = MEM_malloc(sizeof(*node_pair));
	node_pair->key = node->key;
	node_pair->value = node->value;
	list[(*next_index)++] = node_pair;
	_red_black_tree_iterator_created_by_inorder_traversal(tree, node->right, next_index, list, number);
}
static void red_black_tree_iterator_created_by_inorder_traversal(RB_Tree tree, int *next_index, 
	void **list, int number)
{
	_red_black_tree_iterator_created_by_inorder_traversal(tree, tree->root, next_index, list, number);
	if (*next_index != number)
	{
		LOG_ERROR("the next_index does not match the number");
		LOG_EXIT();
	}
	return;
}
RB_Tree_Iterator red_black_tree_iterator_create(RB_Tree tree)
{
	RB_Tree_Iterator iterator = MEM_malloc(sizeof(*iterator));
	iterator->tree = tree;
	iterator->number = tree->number;
	iterator->next_index = 0;
	iterator->list = MEM_malloc(sizeof(iterator->list) * iterator->number);
	int count = 0;
	red_black_tree_iterator_created_by_inorder_traversal(iterator->tree, &count, 
		iterator->list, iterator->number);
	return iterator;
}
void red_black_tree_iterator_free(RB_Tree_Iterator iterator)
{
	for (int i = 0; i < iterator->number; i++)
	{
		MEM_free(iterator->list[i]); 
	}
	MEM_free(iterator->list);
	MEM_free(iterator);
}
RB_Tree_Node_Pair red_black_tree_iterator_get_next(RB_Tree_Iterator iterator)
{
	RB_Tree_Node_Pair pair = NULL;
	if (iterator->next_index < iterator->number)
	{
		pair = iterator->list[iterator->next_index++];
	}
	return pair;
}

void *red_black_tree_node_pair_get_key(RB_Tree_Node_Pair pair)
{
	return pair->key;
}

void *red_black_tree_node_pair_get_value(RB_Tree_Node_Pair pair)
{
	return pair->value;
}

Map map_create(int(*key_compare)(void* k1, void* k2),
	void(*key_free)(void* k),
	void(*value_free)(void* v),
	void(*key_print)(void* k),
	void(*value_print)(void *v))
{
	return red_black_tree_create(key_compare, key_free, value_free, key_print, value_print);
}
void map_free(Map m)
{
	red_black_tree_free(m);
}
void map_insert(Map m, void *k, void *v)
{
	red_black_tree_insert(m, k, v);
}
void *map_search(Map m, void *key)
{
	return red_black_tree_search(m, key);
}
void *map_pop(Map m, void *k)
{
	return red_black_tree_pop(m, k);
}
void map_remove(Map m, void *k)
{
	red_black_tree_remove(m, k);
}
void map_delete(Map m, void *k)
{
	red_black_tree_delete(m, k);
}
void map_print(Map m)
{
	red_black_tree_print(m);
}
int map_get_number(Map m)
{
	return red_black_tree_get_number(m);
}
Map_Iterator map_iterator_create(Map m)
{
	return red_black_tree_iterator_create(m);
}
void map_iterator_free(Map_Iterator mi)
{
	red_black_tree_iterator_free(mi);
}
Map_Node_Pair map_iterator_get_next(Map_Iterator mi)
{
	return red_black_tree_iterator_get_next(mi);
}

void *map_node_pair_get_key(Map_Node_Pair pair)
{
	return red_black_tree_node_pair_get_key(pair);
}
void *map_node_pair_get_value(Map_Node_Pair pair)
{
	return red_black_tree_node_pair_get_value(pair);
}