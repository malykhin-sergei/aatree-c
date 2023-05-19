/* The MIT License (MIT)
Copyright (c) 2023 Sergei Malykhin

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "aatree.h"

aatree_node_t *aatree_prev_node(aatree_node_t *node)
{
  if (node->left)
  {
    for (node = node->left; node->right; node = node->right)
    {
    }
  }
  else
  {
    for (; node; node = node->parent)
    {
      if (!node->parent || (node->parent->right == node))
      {
        node = node->parent;
        break;
      }
    }
  }

  return node;
} /* aatree_prev_node */

aatree_node_t *aatree_next_node(aatree_node_t *node)
{
  if (node->right)
  {
    for (node = node->right; node->left; node = node->left)
    {
    }
  }
  else
  {
    for (; node; node = node->parent)
    {
      if (!node->parent || (node->parent->left == node))
      {
        node = node->parent;
        break;
      }
    }
  }

  return node;
} /* aatree_next_node */

/* Find the node in the AA tree with the supplied key */
static __nonnull((1)) aatree_node_t *aatree_find(const aatree_t *tree,
                                                 const void     *key)
{
  aatree_node_t *node = tree->root;

  while (node)
  {
    /* Do keys comparison to decide whether to go left or right. */
    int result = tree->cmp(key, aatree_node_key(tree, node));

    if (result > 0)
    {
      node = node->right;
    }
    else if (result < 0)
    {
      node = node->left;
    }
    else /* Found the requested node. */
    {
      return node;
    }
  }

  /* Requested node was not found. */
  return NULL;
} /* aatree_find */

/* Find the successor node to the supplied key in the AA tree */
static __nonnull((1)) aatree_node_t *aatree_find_successor(const aatree_t *tree,
                                                           const void     *key,
                                                           int equal)
{
  aatree_node_t *node = tree->root;

  while (node)
  {
    /* Do keys comparison to decide whether to go left or right. */
    int result = tree->cmp(key, aatree_node_key(tree, node));

    if (result > 0)
    {
      if (!node->right)
      {
        return aatree_next_node(node);
      }
      else
      {
        node = node->right;
      }
    }
    else if (result < 0)
    {
      if (!node->left)
      {
        return node;
      }
      else
      {
        node = node->left;
      }
    }
    else
    {
      return equal ? node : aatree_next_node(node);
    }
  }

  return node;
} /* aatree_find_successor */

/* Find the predecessor node to the supplied key in the AA tree */
static __nonnull((1)) aatree_node_t
    *aatree_find_predecessor(const aatree_t *tree, const void *key, int equal)
{
  aatree_node_t *node = tree->root;

  while (node)
  {
    /* Do keys comparison to decide whether to go left or right. */
    int result = tree->cmp(key, aatree_node_key(tree, node));

    if (result < 0)
    {
      if (!node->left)
      {
        return aatree_prev_node(node);
      }
      else
      {
        node = node->left;
      }
    }
    else if (result > 0)
    {
      if (!node->right)
      {
        return node;
      }
      else
      {
        node = node->right;
      }
    }
    else
    {
      return equal ? node : aatree_prev_node(node);
    }
  }

  return node;
} /* aatree_find_predecessor */

void *
aatree_search(const aatree_t *tree, const void *key, aatree_keys_order order)
{
  aatree_node_t *node = NULL;

  switch (order)
  {
    case AATREE_KEY_LT:
    case AATREE_KEY_LE:
      node = aatree_find_predecessor(tree, key, order == AATREE_KEY_LE);
      break;

    case AATREE_KEY_GT:
    case AATREE_KEY_GE:
      node = aatree_find_successor(tree, key, order == AATREE_KEY_GE);
      break;

    case AATREE_KEY_EQ:
    default:
      node = aatree_find(tree, key);
  }

  return aatree_node_entry(tree, node);
} /* aatree_search */

/* 
 *     N        L
 *    / \      / \
 *   L   R => A   N
 *  / \          / \
 * A   B        B   R
 *
 * Skew is a right rotation to replace a subtree containing a left horizontal
 * link with one containing a right horizontal link instead.
 **/
static __inline__ __nonnull((1)) void skew(aatree_t *tree, aatree_node_t *node)
{
  if (node && node->left && (node->left->level == node->level))
  {
    aatree_node_t *left = node->left;
    node->left          = left->right;

    if (node->left)
    {
      node->left->parent = node;
    }

    left->right  = node;
    left->parent = node->parent;
    node->parent = left;

    if (left->parent)
    {
      if (left->parent->left == node)
      {
        left->parent->left = left;
      }
      else
      {
        left->parent->right = left;
      }
    }
    else
    {
      tree->root = left;
    }

    node = left;
  }
} /* skew */

/* 
 *   N          R
 *  / \        / \
 * A   R  =>  N   X
 *    / \    / \
 *   B   X  A   B
 *
 * Split is a left rotation and level increase to replace a subtree containing
 * two or more consecutive right horizontal links with one containing two
 * fewer consecutive right horizontal links.
 **/
static __inline__ __nonnull((1)) void split(aatree_t *tree, aatree_node_t *node)
{
  if (node && node->right && node->right->right
      && (node->level == node->right->right->level))
  {
    aatree_node_t *right = node->right;
    node->right          = right->left;

    if (node->right)
      node->right->parent = node;

    right->left   = node;
    right->parent = node->parent;
    node->parent  = right;

    right->level += 1;

    if (right->parent)
    {
      if (right->parent->left == node)
      {
        right->parent->left = right;
      }
      else
      {
        right->parent->right = right;
      }
    }
    else
    {
      tree->root = right;
    }

    node = right;
  }
} /* split */

/* Adjust node level */
static __inline__ __nonnull((1)) void decrease_level(aatree_node_t *node)
{
  int should_be = aatree_node_level(node);

  if (node->level > should_be)
  {
    node->level = should_be;

    if (node->right && (node->right->level > should_be))
    {
      node->right->level = should_be;
    }
  }
} /* decrease_level */

void *aatree_insert(aatree_t *tree, aatree_node_t *node)
{
  aatree_node_t *parent_node = NULL;

  node->level = 1;

  if (!tree->root)
  {
    /* Tree is empty, so insert at root. */
    tree->root  = node;
    tree->first = node;
    tree->last  = node;
    return NULL;
  }

  /* Scan down the tree looking for the appropriate insert point. */
  for (parent_node = tree->root; parent_node;)
  {
    /* Do keys comparison to decide whether to go left or right. */
    int result = tree->cmp(aatree_node_key(tree, node),
                           aatree_node_key(tree, parent_node));

    if (result > 0) /* Move down right subtree. */
    {
      if (!parent_node->right) /* Subtree is empty, so insert here. */
      {
        node->parent       = parent_node;
        parent_node->right = node;

        if (parent_node == tree->last)
        {
          tree->last = node;
        }

        break;
      }
      else /* Subtree is not empty. */
      {
        parent_node = parent_node->right;
      }
    }
    else if (result < 0) /* Move down left subtree. */
    {
      if (!parent_node->left) /* Subtree is empty, so insert here. */
      {
        node->parent      = parent_node;
        parent_node->left = node;

        if (parent_node == tree->first)
        {
          tree->first = node;
        }

        break;
      }
      else /* Subtree is not empty */
      {
        parent_node = parent_node->left;
      }
    }
    else /* Found a matching key, return entry found. */
    {
      return aatree_node_entry(tree, parent_node);
    }
  }

  /* Perform skew and then split. The conditionals that determine whether or
   * not a rotation will occur or not are inside of the procedures, as given
   * above. */
  for (; parent_node; parent_node = parent_node->parent)
  {
    skew(tree, parent_node);
    split(tree, parent_node);
  }

  return NULL;
} /* aatree_insert */

void aatree_delete(aatree_t *tree, aatree_node_t *node)
{
  aatree_node_t *parent_node = NULL;

  /* Case I. Node is a leaf. */
  if (!node->left && !node->right)
  {
    if (!node->parent)
    {
      /* In this case last node is to be deleted and the tree becomes empty. */
      if (tree->root == node)
      {
        tree->root  = NULL;
        tree->first = NULL;
        tree->last  = NULL;
        node->level = 0;
      }
    }
    else if (node->parent->right == node)
    {
      if (tree->last == node)
      {
        tree->last = node->parent;
      }

      parent_node        = node->parent;
      parent_node->right = NULL;
    }
    else
    {
      if (tree->first == node)
      {
        tree->first = node->parent;
      }

      parent_node       = node->parent;
      parent_node->left = NULL;
    }
  }
  /* Case II. Node has only one son. */
  else if (!node->left && node->right)
  {
    if (!node->parent)
    {
      /* If the node to be deleted has only one son and no parent -
       * the tree has only one node remaining. */
      tree->root  = node->right;
      tree->first = node->right;
      tree->last  = node->right;
    }
    else if (node->parent->right == node)
    {
      node->parent->right = node->right;
    }
    else
    {
      if (tree->first == node)
      {
        tree->first = node->right;
      }

      node->parent->left = node->right;
    }

    parent_node         = node->right;
    parent_node->parent = node->parent;
  }
  /* Case III. Node has two sons. */
  else
  {
    aatree_node_t *successor = node->right;

    if (!successor->left)
    {
      parent_node        = successor;
      parent_node->left  = node->left;
      node->left->parent = parent_node;

      if (!node->parent)
      {
        /* Node must be a root! */
        tree->root = parent_node;
      }
      else if (node->parent->right == node)
      {
        node->parent->right = parent_node;
      }
      else
      {
        node->parent->left = parent_node;
      }

      parent_node->parent = node->parent;
      parent_node->level  = node->level;
    }
    else
    {
      /* Find the leftmost node in the right subtree */
      for (; successor->left; successor = successor->left)
      {
      }

      parent_node       = successor->parent;
      parent_node->left = successor->right;

      if (successor->right)
      {
        successor->right->parent = parent_node;
      }

      successor->left    = node->left;
      node->left->parent = successor;

      successor->right    = node->right;
      node->right->parent = successor;

      if (!node->parent)
      {
        /* Node must be a root! */
        tree->root = successor;
      }
      else if (node->parent->right == node)
      {
        node->parent->right = successor;
      }
      else
      {
        node->parent->left = successor;
      }

      successor->parent = node->parent;
      successor->level  = node->level;
    }
  }

  /* Rebalance the tree. Decrease the level of all nodes in this level if
   * necessary, and then skew and split all nodes in the new level. */
  for (; parent_node; parent_node = parent_node->parent)
  {
    decrease_level(parent_node);
    skew(tree, parent_node);
    skew(tree, parent_node->right);

    if (parent_node->right)
    {
      skew(tree, parent_node->right->right);
    }

    split(tree, parent_node);
    split(tree, parent_node->right);
  }

  /* Unlink deleted node. */
  aatree_init_node(node);
} /* aatree_delete */
