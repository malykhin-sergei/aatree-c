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

#ifndef AATREE_H
#define AATREE_H

#include <stddef.h>
#include <stdint.h>

typedef enum aatree_keys_order_e
{
  AATREE_KEY_EQ,
  AATREE_KEY_LT,
  AATREE_KEY_LE,
  AATREE_KEY_GT,
  AATREE_KEY_GE
} aatree_keys_order;

/* The comparison function must return an integer less than, equal to, or
 * greater than zero if the first argument is considered to be respectively
 * less than, equal to, or greater than the second.
 */
typedef int(aatree_keys_compare)(const void *, const void *);

/* AA tree node. */
typedef struct aatree_node
{
  struct aatree_node *parent;
  struct aatree_node *left;
  struct aatree_node *right;

  /* height of a corresponding node in the 2-3 tree */
  uint8_t level;
} aatree_node_t;

/* AA tree */
typedef struct aatree
{
  aatree_node_t *root;
  aatree_node_t *first;
  aatree_node_t *last;

  /* Offsets to node and key */
  struct offset
  {
    uint16_t key;
    uint16_t node;
  } offset;

  /* Keys comparison function */
  aatree_keys_compare *cmp;
} aatree_t;

/* Init empty AA tree */
static __inline__ __nonnull((1)) void aatree_init_tree(aatree_t *tree,
                                                       uint16_t  node_offset,
                                                       uint16_t  key_offset,
                                                       aatree_keys_compare cmp)
{
  tree->first = NULL;
  tree->root  = NULL;
  tree->last  = NULL;

  tree->offset.node = node_offset;
  tree->offset.key  = key_offset;

  tree->cmp = cmp;
} /* aatree_init_tree */

/* Init AA tree node */
static __inline__ __nonnull((1)) void aatree_init_node(aatree_node_t *node)
{
  node->parent = NULL;
  node->left   = NULL;
  node->right  = NULL;
  node->level  = 0;
} /* aatree_init_node */

/* Get pointer to the entry (container) from a tree node */
static __inline__ __nonnull((1)) void *aatree_node_entry(const aatree_t *tree,
                                                         aatree_node_t  *node)
{
  return node ? (void *)((uint8_t *)node - tree->offset.node) : NULL;
} /* aatree_node_entry */

/* Get pointer to the tree node from an entry (container) */
static __inline__ __nonnull((1))
    aatree_node_t *aatree_entry_node(const aatree_t *tree, void *entry)
{
  return entry ? (aatree_node_t *)((uint8_t *)entry + tree->offset.node) : NULL;
} /* aatree_entry_node */

/* Get pointer to the entry's key from a tree node */
static __inline__ __nonnull((1)) void *aatree_node_key(const aatree_t *tree,
                                                       aatree_node_t  *node)
{
  return (uint8_t *)node - tree->offset.node + tree->offset.key;
} /* aatree_node_key */

/* Get pointer to the entry's key from an entry (container) */
static __inline__ __nonnull((1)) void *aatree_entry_key(const aatree_t *tree,
                                                        void           *entry)
{
  return (uint8_t *)entry + tree->offset.key;
} /* aatree_entry_key */

/* Calculate AA tree node level */
static __inline__ __nonnull((1)) int aatree_node_level(aatree_node_t *node)
{
  int level_left  = node->left ? node->left->level : 0;
  int level_right = node->right ? node->right->level : 0;
  int level       = level_left < level_right ? level_left + 1 : level_right + 1;

  return level;
} /* aatree_node_level */

/* Find the first entry in the AA tree */
static __inline__ __nonnull((1)) void *aatree_first(const aatree_t *tree)
{
  return aatree_node_entry(tree, tree->first);
} /* aatree_first */

/* Find the last entry in the AA tree */
static __inline__ __nonnull((1)) void *aatree_last(const aatree_t *tree)
{
  return aatree_node_entry(tree, tree->last);
} /* aatree_last */

/* Trace the origin of a node (tree root) following from parent to parent */
static __inline__ __nonnull((1))
    aatree_node_t *aatree_node_get_root(aatree_node_t *node)
{
  if (!node->level)
    return NULL;

  for (; node->parent; node = node->parent)
  {
  }

  return node;
} /* aatree_node_get_root */

/* Get previous node of a tree */
aatree_node_t *aatree_prev_node(aatree_node_t *node) __nonnull((1));

/* Get previous entry of the AA tree */
static __inline__ __nonnull((1, 2)) void *aatree_prev(const aatree_t *tree,
                                                      aatree_node_t  *node)
{
  return aatree_node_entry(tree, aatree_prev_node(node));
} /* aatree_prev */

/* Get next node of a tree */
aatree_node_t *aatree_next_node(aatree_node_t *node) __nonnull((1));

/* Get next entry of the AA tree */
static __inline__ __nonnull((1, 2)) void *aatree_next(const aatree_t *tree,
                                                      aatree_node_t  *node)
{
  return aatree_node_entry(tree, aatree_next_node(node));
} /* aatree_next */

/* Search node with a key equal to, less or greater than the key provided */
void *aatree_search(const aatree_t   *tree,
                    const void       *key,
                    aatree_keys_order order) __nonnull((1));

/* Try to insert node into tree or return an existing entry */
void *aatree_insert(aatree_t *tree, aatree_node_t *node) __nonnull((1, 2));

/* Delete specified node from tree */
void aatree_delete(aatree_t *tree, aatree_node_t *node) __nonnull((1, 2));

/* Verify AA tree sructure */
int aatree_verify(aatree_t *tree);

#endif /* AATREE_H */