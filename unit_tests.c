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

#include <stdlib.h>
#include "aatree.h"
#include "utest.h"

#define COUNT 127

typedef struct number
{
  aatree_node_t node;
  int           value;
} number_t;

static int cmp_ints(const void *a, const void *b)
{
  int key_a = *((int *)a);
  int key_b = *((int *)b);

  if (key_a > key_b)
    return 1;
  else if (key_a < key_b)
    return -1;
  else
    return 0;
}

static void shuffle(int *arr, size_t n)
{
  for (int i = 0; i < n - 1; i++)
  {
    int j = i + rand() / (RAND_MAX / (n - i) + 1);
    int t = arr[j];

    arr[j] = arr[i];
    arr[i] = t;
  }
}

UTEST(aatree, init)
{
  aatree_t tree;
  number_t x;

  aatree_init_tree(&tree, offsetof(number_t, node), offsetof(number_t, value),
                   cmp_ints);

  ASSERT_TRUE(tree.root == NULL);
  ASSERT_TRUE(tree.first == NULL);
  ASSERT_TRUE(tree.last == NULL);
  ASSERT_TRUE(tree.cmp == cmp_ints);
  ASSERT_TRUE(tree.offset.key == offsetof(number_t, value));
  ASSERT_TRUE(tree.offset.node == offsetof(number_t, node));

  aatree_init_node(&x.node);

  ASSERT_EQ(aatree_node_entry(&tree, &x.node), &x);
  ASSERT_EQ(aatree_entry_node(&tree, &x), &x.node);

  x.value = 1;
  ASSERT_EQ(*(int *)aatree_node_key(&tree, &x.node), 1);
  x.value = 2;
  ASSERT_EQ(*(int *)aatree_node_key(&tree, &x.node), 2);

  ASSERT_EQ(*(int *)aatree_entry_key(&tree, &x), 2);
  x.value = 3;
  ASSERT_EQ(*(int *)aatree_entry_key(&tree, &x), 3);

  ASSERT_EQ(aatree_node_level(&x.node), 1);
}

UTEST(aatree, single_item)
{
  aatree_t tree;
  number_t x;
  int      key = 1;

  aatree_init_tree(&tree, offsetof(number_t, node), offsetof(number_t, value),
                   cmp_ints);

  aatree_init_node(&x.node);
  x.value = 1;

  ASSERT_TRUE(aatree_insert(&tree, &x.node) == NULL);
  ASSERT_TRUE(aatree_node_level(&x.node) == 1);
  ASSERT_TRUE(aatree_first(&tree) == &x);
  ASSERT_TRUE(aatree_last(&tree) == &x);
  ASSERT_TRUE(aatree_node_get_root(&x.node) == &x.node);
  ASSERT_TRUE(aatree_prev(&tree, &x.node) == NULL);
  ASSERT_TRUE(aatree_next(&tree, &x.node) == NULL);

  ASSERT_TRUE(aatree_search(&tree, &key, AATREE_KEY_EQ) == &x);
  ASSERT_TRUE(aatree_search(&tree, &key, AATREE_KEY_LE) == &x);
  ASSERT_TRUE(aatree_search(&tree, &key, AATREE_KEY_LT) == NULL);
  ASSERT_TRUE(aatree_search(&tree, &key, AATREE_KEY_GE) == &x);
  ASSERT_TRUE(aatree_search(&tree, &key, AATREE_KEY_GT) == NULL);

  key = 2;

  ASSERT_TRUE(aatree_search(&tree, &key, AATREE_KEY_EQ) == NULL);
  ASSERT_TRUE(aatree_search(&tree, &key, AATREE_KEY_LE) == &x);
  ASSERT_TRUE(aatree_search(&tree, &key, AATREE_KEY_LT) == &x);
  ASSERT_TRUE(aatree_search(&tree, &key, AATREE_KEY_GE) == NULL);
  ASSERT_TRUE(aatree_search(&tree, &key, AATREE_KEY_GT) == NULL);

  key = -2;

  ASSERT_TRUE(aatree_search(&tree, &key, AATREE_KEY_EQ) == NULL);
  ASSERT_TRUE(aatree_search(&tree, &key, AATREE_KEY_LE) == NULL);
  ASSERT_TRUE(aatree_search(&tree, &key, AATREE_KEY_LT) == NULL);
  ASSERT_TRUE(aatree_search(&tree, &key, AATREE_KEY_GE) == &x);
  ASSERT_TRUE(aatree_search(&tree, &key, AATREE_KEY_GT) == &x);

  aatree_delete(&tree, &x.node);

  ASSERT_TRUE(x.node.left == NULL);
  ASSERT_TRUE(x.node.right == NULL);
  ASSERT_TRUE(x.node.parent == NULL);

  ASSERT_TRUE(aatree_node_get_root(&x.node) == NULL);
  ASSERT_TRUE(aatree_first(&tree) == NULL);
  ASSERT_TRUE(aatree_last(&tree) == NULL);
}

UTEST(aatree, two_items)
{
  aatree_t tree;
  number_t x, y;

  int key = 1;

  aatree_init_tree(&tree, offsetof(number_t, node), offsetof(number_t, value),
                   cmp_ints);

  aatree_init_node(&x.node);
  aatree_init_node(&y.node);

  x.value = 1;
  y.value = 1;

  ASSERT_TRUE(aatree_insert(&tree, &x.node) == NULL);
  ASSERT_TRUE(aatree_insert(&tree, &y.node) == &x);

  y.value = 2;
  ASSERT_TRUE(aatree_insert(&tree, &y.node) == NULL);

  ASSERT_TRUE(x.node.parent == NULL);
  ASSERT_TRUE(x.node.right == &y.node);
  ASSERT_TRUE(x.node.left == NULL);

  ASSERT_TRUE(y.node.parent == &x.node);
  ASSERT_TRUE(y.node.left == NULL);
  ASSERT_TRUE(y.node.right == NULL);

  ASSERT_TRUE(aatree_search(&tree, &key, AATREE_KEY_EQ) == &x);
  key = 2;
  ASSERT_TRUE(aatree_search(&tree, &key, AATREE_KEY_EQ) == &y);

  ASSERT_TRUE(aatree_first(&tree) == &x);
  ASSERT_TRUE(aatree_last(&tree) == &y);

  aatree_delete(&tree, &x.node);

  ASSERT_TRUE(aatree_first(&tree) == &y);
  ASSERT_TRUE(aatree_last(&tree) == &y);

  ASSERT_TRUE(aatree_insert(&tree, &x.node) == NULL);
  ASSERT_TRUE(aatree_next(&tree, &x.node) == &y);
  ASSERT_TRUE(aatree_prev(&tree, &y.node) == &x);
}

UTEST(aatree, sequence)
{
  aatree_t tree;
  number_t num[COUNT];
  int      ix[COUNT];

  aatree_init_tree(&tree, offsetof(number_t, node), offsetof(number_t, value),
                   cmp_ints);

  for (int i = 0; i < COUNT; i++)
  {
    ix[i]        = i;
    num[i].value = i;
    aatree_init_node(&num[i].node);
  }

  shuffle(ix, COUNT);

  for (int i = 0; i < COUNT; i++)
  {
    ASSERT_TRUE(aatree_insert(&tree, &num[ix[i]].node) == NULL);
  }

  ASSERT_EQ(aatree_verify(&tree), EXIT_SUCCESS);

  shuffle(ix, COUNT);

  for (int i = 0; i < COUNT - 1; i++)
  {
    aatree_delete(&tree, &num[ix[i]].node);
  }

  int remain = ix[COUNT - 1];

  ASSERT_TRUE(tree.root == &num[remain].node);
  ASSERT_TRUE(aatree_first(&tree) == &num[remain].node);
  ASSERT_TRUE(aatree_last(&tree) == &num[remain].node);
}

UTEST(aatree, search)
{
  aatree_t tree;
  number_t num[COUNT];
  int      ix[COUNT];

  aatree_init_tree(&tree, offsetof(number_t, node), offsetof(number_t, value),
                   cmp_ints);

  for (int i = 0; i < COUNT; i++)
  {
    ix[i]        = i;
    num[i].value = i;
    aatree_init_node(&num[i].node);
  }

  shuffle(ix, COUNT);

  for (int i = 0; i < COUNT; i++)
  {
    aatree_insert(&tree, &num[ix[i]].node);
  }

  for (int i = 0; i < COUNT; i++)
  {
    number_t *x = aatree_search(&tree, &i, AATREE_KEY_EQ);

    ASSERT_NE(x, NULL);

    if (x)
    {
      ASSERT_EQ(x->value, i);
      ASSERT_EQ(aatree_node_get_root(&x->node), tree.root);
    }
  }
}

UTEST(aatree, backward)
{
  aatree_t tree;
  number_t num[COUNT];
  int      ix[COUNT];
  int      ii = COUNT - 1;

  aatree_init_tree(&tree, offsetof(number_t, node), offsetof(number_t, value),
                   cmp_ints);

  for (int i = 0; i < COUNT; i++)
  {
    ix[i]        = i;
    num[i].value = i;
    aatree_init_node(&num[i].node);
  }

  shuffle(ix, COUNT);

  for (int i = 0; i < COUNT; i++)
  {
    aatree_insert(&tree, &num[ix[i]].node);
  }

  for (number_t *x = aatree_last(&tree); x != NULL;
       x           = aatree_prev(&tree, &x->node))
  {
    ASSERT_EQ(x->value, ii);
    ii--;
  }

  for (number_t *x = aatree_first(&tree); x != NULL; x = aatree_first(&tree))
  {
    aatree_delete(&tree, &x->node);
  }

  for (int i = 0; i < COUNT; i++)
  {
    ASSERT_EQ(num[i].node.level, 0);
    ASSERT_EQ(num[i].node.parent, NULL);
    ASSERT_EQ(num[i].node.right, NULL);
    ASSERT_EQ(num[i].node.left, NULL);
  }

  ASSERT_EQ(aatree_search(&tree, &ii, AATREE_KEY_GE), NULL);
  ASSERT_EQ(aatree_search(&tree, &ii, AATREE_KEY_LE), NULL);
}

UTEST(aatree, root)
{
  aatree_t tree;
  number_t num[COUNT];
  int      ix[COUNT];

  aatree_init_tree(&tree, offsetof(number_t, node), offsetof(number_t, value),
                   cmp_ints);

  for (int i = 0; i < COUNT; i++)
  {
    ix[i]        = i;
    num[i].value = i;
    aatree_init_node(&num[i].node);
  }

  shuffle(ix, COUNT);

  for (int i = 0; i < COUNT; i++)
  {
    aatree_insert(&tree, &num[ix[i]].node);
  }

  for (aatree_node_t *x = tree.root; x != NULL; x = tree.root)
  {
    aatree_delete(&tree, x);
  }

  for (int i = 0; i < COUNT; i++)
  {
    ASSERT_EQ(num[i].node.level, 0);
    ASSERT_EQ(num[i].node.parent, NULL);
    ASSERT_EQ(num[i].node.right, NULL);
    ASSERT_EQ(num[i].node.left, NULL);
  }
}

UTEST(aatree, first)
{
  aatree_t tree;
  number_t num[COUNT];
  int      ix[COUNT];

  aatree_init_tree(&tree, offsetof(number_t, node), offsetof(number_t, value),
                   cmp_ints);

  for (int i = 0; i < COUNT; i++)
  {
    ix[i]        = i;
    num[i].value = i;
    aatree_init_node(&num[i].node);
  }

  shuffle(ix, COUNT);

  for (int i = 0; i < COUNT; i++)
  {
    aatree_insert(&tree, &num[ix[i]].node);
  }

  for (aatree_node_t *x = tree.first; x != NULL; x = tree.first)
  {
    aatree_delete(&tree, x);
  }

  for (int i = 0; i < COUNT; i++)
  {
    ASSERT_EQ(num[i].node.level, 0);
    ASSERT_EQ(num[i].node.parent, NULL);
    ASSERT_EQ(num[i].node.right, NULL);
    ASSERT_EQ(num[i].node.left, NULL);
  }
}

UTEST(aatree, last)
{
  aatree_t tree;
  number_t num[COUNT];
  int      ix[COUNT];

  aatree_init_tree(&tree, offsetof(number_t, node), offsetof(number_t, value),
                   cmp_ints);

  for (int i = 0; i < COUNT; i++)
  {
    ix[i]        = i;
    num[i].value = i;
    aatree_init_node(&num[i].node);
  }

  shuffle(ix, COUNT);

  for (int i = 0; i < COUNT; i++)
  {
    aatree_insert(&tree, &num[ix[i]].node);
  }

  for (aatree_node_t *x = tree.last; x != NULL; x = tree.last)
  {
    aatree_delete(&tree, x);
  }

  for (int i = 0; i < COUNT; i++)
  {
    ASSERT_EQ(num[i].node.level, 0);
    ASSERT_EQ(num[i].node.parent, NULL);
    ASSERT_EQ(num[i].node.right, NULL);
    ASSERT_EQ(num[i].node.left, NULL);
  }
}

UTEST(aatree, search_forward)
{
  aatree_t tree;
  number_t num[COUNT];
  int      ix[COUNT];

  aatree_init_tree(&tree, offsetof(number_t, node), offsetof(number_t, value),
                   cmp_ints);

  for (int i = 0; i < COUNT; i++)
  {
    ix[i]        = i;
    num[i].value = i;
    aatree_init_node(&num[i].node);
  }

  shuffle(ix, COUNT);

  for (int i = 0; i < COUNT; i++)
  {
    aatree_insert(&tree, &num[ix[i]].node);
  }

  for (int i = -1; i < COUNT - 1; i++)
  {
    number_t *x = aatree_search(&tree, &i, AATREE_KEY_GT);

    ASSERT_NE(x, NULL);

    if (x)
    {
      ASSERT_EQ(x->value, i + 1);
    }
  }
}

UTEST(aatree, search_backward)
{
  aatree_t tree;
  number_t num[COUNT];
  int      ix[COUNT];

  aatree_init_tree(&tree, offsetof(number_t, node), offsetof(number_t, value),
                   cmp_ints);

  for (int i = 0; i < COUNT; i++)
  {
    ix[i]        = i;
    num[i].value = i;
    aatree_init_node(&num[i].node);
  }

  shuffle(ix, COUNT);

  for (int i = 0; i < COUNT; i++)
  {
    aatree_insert(&tree, &num[ix[i]].node);
  }

  for (int i = 1; i < COUNT + 1; i++)
  {
    number_t *x = aatree_search(&tree, &i, AATREE_KEY_LT);

    ASSERT_NE(x, NULL);

    if (x)
    {
      ASSERT_EQ(x->value, i - 1);
    }
  }
}

UTEST_MAIN();
