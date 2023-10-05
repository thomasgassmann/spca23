#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef struct tree_node
{
  int key;
  int value;
  struct tree_node *left;
  struct tree_node *right;
} tree_node;

tree_node *insert(tree_node *root, int key, int value);
tree_node *lookup(tree_node *root, int key);
tree_node *delete(tree_node *root, int key);

struct withp
{
  tree_node *parent;
  tree_node *res;
};

struct withp explore(tree_node *root, int key)
{
  tree_node *current = root;
  tree_node *parent = NULL;
  while (current != NULL && current->value != key)
  {
    parent = current;
    if (key < current->value)
    {
      current = current->left;
    }
    else
    {
      current = current->right;
    }
  }

  struct withp res;
  res.parent = parent;
  res.res = current;
  return res;
}

// finds successor if we delete root
struct withp right_succ(tree_node *root)
{
  struct withp res;
  res.parent = root;
  res.res = root->right;
  while (res.res->left != NULL)
  {
    res.parent = res.res;
    res.res = res.res->left;
  }

  return res;
}

tree_node *insert(tree_node *root, int key, int value)
{
  struct withp i = explore(root, key);
  if (i.res != NULL)
  {
    return root;
  }

  tree_node *new_node = calloc(sizeof(tree_node), 1);
  new_node->key = key;
  new_node->value = value;
  if (root == NULL)
  {
    return new_node;
  }

  if (i.parent->key > key)
  {
    i.parent->left = new_node;
  }
  else
  {
    i.parent->right = new_node;
  }

  return root;
}

tree_node *lookup(tree_node *root, int key)
{
  struct withp i = explore(root, key);
  return i.res;
}

void replace_child(tree_node *parent, tree_node *old, tree_node *new) {
  if (parent->left == old) {
    parent->left = new;
  } else {
    parent->right = new;
  }
}

tree_node *delete(tree_node *root, int key)
{
  struct withp i = explore(root, key);
  if (i.res == NULL)
  {
    // key does not exist
    return root;
  }

  if (i.res->left == NULL && i.res->right == NULL)
  {
    if (i.parent == NULL)
    {
      // delete the entire tree
      // user asks to delete root, root has no children
      free(i.res);
      return NULL;
    }

    replace_child(i.parent, i.res, NULL);
    free(i.res);
    return root;
  }

  if ((i.res->left == NULL) ^ (i.res->right == NULL))
  {
    tree_node *replace = i.res->left == NULL ? i.res->right : i.res->left;
    if (i.parent == NULL)
    {
      // delete root, new root is the right child
      tree_node *new_root = replace;
      free(i.res);
      return new_root;
    }

    replace_child(i.parent, i.res, replace);
    free(i.res);
    return root;
  }

  struct withp successor = right_succ(i.res);

  // successor.res->left is null
  if (successor.parent == i.res) {
    // successor is immediate right child of res
    successor.res->left = i.res->left;
    if (i.parent == NULL) {
      free(i.res);
      return successor.res;
    }

    replace_child(i.parent, i.res, successor.res);
    free(i.res);
    return root;
  }

  successor.parent->left = successor.res->right;
  successor.res->right = NULL;

  successor.res->left = i.res->left;
  successor.res->right = i.res->right;

  if (i.parent == NULL) {
    free(i.res);
    return successor.res;
  } else {
    replace_child(i.parent, i.res, successor.res);
    free(i.res);
    return root;
  }
}

int main()
{
  srand(10);
  const int SIZE = 10000;
  int random[SIZE];
  for (int i = 0; i < SIZE; i++) {
    random[i] = rand();
  }

  tree_node *root = NULL;
  for (int i = 0; i < SIZE; i++) {
    root = insert(root, random[i], random[i]);
    tree_node *f = lookup(root, random[i]);
    assert(f != NULL && f->key == random[i]);
  }

  for (int i = 0; i < SIZE; i++) {
    tree_node *f = lookup(root, random[i]);
    assert(f != NULL && f->key == random[i]);
  }

  int start = rand() % SIZE;
  for (int i = (start + 1) % SIZE; i != start; i = (i + 1) % SIZE) {
    root = delete(root, random[i]);
    
    tree_node *f = lookup(root, random[i]);
    assert(f == NULL);
  }

  assert(root != NULL);
  assert(root->left == NULL);
  assert(root->right == NULL);
  assert(root->key == random[start]);
  printf("%p\n", root);
}
