#include <stdlib.h>
#include <stdio.h>

#include "bst.h"

// Local functions
static int place_bst_node( struct BST_Node *parent, struct BST_Node *node );
static struct BST_Node *make_bst_node( int key, void *data );

// Root's pointer is passed because root can get modified for the first node
int bst_add_node( struct BST_Node **root, int key, void *data )
{
	struct BST_Node *newnode = NULL;
	struct BST_Node *parent = NULL;
	struct BST_Node *retnode = NULL;
	int status = 0;

	newnode = make_bst_node( key, data);
	if( *root == NULL ){
		*root = newnode;
		status = BST_SUCCESS;
	}
	else{
		status = place_bst_node( *root, newnode );
	}
	return status;
}

struct BST_Node *bst_search( struct BST_Node *root, int key )
{
	struct BST_Node *retval = NULL;

	if( root == NULL ){
		return NULL;
	}
	else if( root->key == key )
		return root;
	else if( key < root->key )
		return bst_search( root->left_child, key );
	else if( key > root->key )
		return bst_search( root->right_child, key );
}
void bst_print( struct BST_Node *root )
{
	if( root == NULL )
		return;
	else{
		printf("%d ", root->key);
		bst_print( root->left_child );
		bst_print( root->right_child );
	}
}

void bst_free( struct BST_Node *root )
{
	if( root == NULL )
		return;
	else{
		bst_free( root->left_child );
		bst_free( root->right_child );
		free(root);
	}
}

void bst_destroy( struct BST_Node *root )
{
	if( root == NULL )
		return;
	else{
		bst_free( root->left_child );
		bst_free( root->right_child );
		free(root->data);
		free(root);
	}
}

static int place_bst_node( struct BST_Node *parent, struct BST_Node *node )
{
	int retstatus;

	if( parent == NULL ){
		return BST_NULL;
	}
	else if( node->key == parent->key ){
		return BST_DUP_KEY;
	}
	else if( node->key < parent->key ){
		if( parent->left_child == NULL ){
			parent->left_child = node;
			return BST_SUCCESS;
		}
		else{
			return place_bst_node( parent->left_child, node );
		}
	}
	else if( node->key > parent->key ){
		if( parent->right_child == NULL ){
			parent->right_child = node;
			return BST_SUCCESS;
		}
		else{
			return place_bst_node( parent->right_child, node );
		}
	}
}

static struct BST_Node *make_bst_node( int key, void *data )
{
	struct BST_Node *newnode;
	newnode = (struct BST_Node *) malloc(sizeof(struct BST_Node));
	newnode->key = key;
	newnode->data = data;
	newnode->left_child = NULL;
	newnode->right_child = NULL;

	return newnode;
}

int flag;
struct BST_Node* delete_node(struct BST_Node *cur,int key)
{
	if(cur==NULL)return NULL;
	if(cur->key<key)
	{
		cur->right_child=delete_node(cur->right_child,key);
		return cur;
	}
	else if(cur->key>key)
	{
		cur->left_child=delete_node(cur->left_child,key);
		return cur;
	}
	flag=1;
	if(cur->left_child==NULL || cur->right_child==NULL)
	{
		if(cur->left_child!=NULL)
		{
			struct BST_Node* tmp=cur->left_child;
			free(cur);
			return tmp;
		}
		if(cur->right_child!=NULL)
		{
			struct BST_Node* tmp=cur->right_child;
			free(cur);
			return tmp;
		}
		free(cur);
		return NULL;
	}
	struct BST_Node* jnext=cur->right_child;
	struct BST_Node* pjnext=cur;
	while(jnext->left_child!=NULL)
	{
		pjnext=jnext;
		jnext=jnext->left_child;
	}
	if(jnext==cur->right_child)
	{
		jnext->left_child=cur->left_child;
		free(cur);
		return jnext;
	}
	pjnext->left_child=jnext->right_child;
	jnext->left_child=cur->left_child;
	jnext->right_child=cur->right_child;
	free(cur);
	return jnext;
}
int bst_del_node( struct BST_Node **root, int key )
{
	flag=0;
	*root=delete_node(*root,key);
	if(flag)
		return BST_SUCCESS;
	else
		return BST_NULL;
}
