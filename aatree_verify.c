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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "aatree.h"

int aatree_verify(aatree_t *tree)
{
  aatree_node_t *node = NULL;
  aatree_node_t *tmp  = NULL;

  int            max      = 0;
  aatree_node_t *max_node = NULL;

  assert(tree != NULL);

  /* Assert that the root node has no parent. */
  assert(!tree->root || !tree->root->parent);

  for (node = tree->first; node; node = node->parent)
  {
    assert(node->left == tmp);
    assert(node != node->parent);
    assert(node != node->left);
    tmp = node;
  }

  assert(tree->root == tmp);

  for (node = tree->first; node; node = tmp)
  {
    int result = -1;

    if (node->level > max)
    {
      max      = node->level;
      max_node = node;
    }

    tmp = aatree_next_node(node);

    if (!tmp)
      break;

    result = tree->cmp(aatree_node_key(tree, node), aatree_node_key(tree, tmp));
    assert(result < 0);

    result = tree->cmp(aatree_node_key(tree, tmp), aatree_node_key(tree, node));
    assert(result > 0);

    /* Check the parentage. */
    assert((node->parent != NULL) || (node == tree->root));

    /* Assert node level is correct. */
    assert(aatree_node_level(node) == node->level);
  }

  /* Assert last node is correct. */
  assert(node == tree->last);

  /* Assert root node has maximum possible level. */
  assert(tree->root == max_node);

  return EXIT_SUCCESS;
} /* aatree_verify */
