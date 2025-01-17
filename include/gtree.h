#ifndef _GTREE_H
#define _GTREE_H

#define I_IN_ORDER			1
#define I_PRE_ORDER			2
#define I_POST_ORDER			3
#define I_LEVEL_ORDER			4

#undef TRUE
#undef FALSE

#define FALSE				0
#define TRUE				! FALSE

typedef void*			ipointer;
typedef const void*		iconstpointer;
typedef int			iint;
typedef int			ires;
typedef int			ITraverseType;

typedef iint (*ICompareFunc)(iconstpointer a, iconstpointer b);
typedef iint (*ITraverseFunc)(ipointer key, ipointer value, ipointer data);
typedef iint (*ISearchFunc)(ipointer key, ipointer data);

typedef struct ITree_Node
{
	iint 				balance;
	ipointer			key;
	ipointer			value;
	
	struct ITree_Node*		parent;
	struct ITree_Node*		left;
	struct ITree_Node*		right;
} ITree_Node;

typedef struct ITree
{
	iint				nnodes;
	iint				height;
	struct ITree_Node*		root;
	ICompareFunc			cmp_func;
} ITree;

ITree*		i_tree_new(ICompareFunc func);
void		i_tree_destroy(ITree *tree);

ires 		i_tree_insert(ITree *tree, ipointer key, ipointer value);
ires		i_tree_remove(ITree *tree, ipointer key);
ipointer 	i_tree_lookup(ITree *tree, ipointer key);
iint 		i_tree_nnodes(ITree *tree);
iint 		i_tree_height(ITree *tree);
void 		i_tree_traverse(ITree *tree, ITraverseFunc traverse_func, ITraverseType traverse_type, ipointer data);

#endif
