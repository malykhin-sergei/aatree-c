# aatree-c

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

[Self-balancing binary tree (AA-tree)](https://en.wikipedia.org/wiki/AA_tree) implemented in pure ANSI C. The implementation is iterative (non-recursive), no memory allocations performed under the hood. It uses an embedded struct `aatree_node_t` that contains 3 pointers (to the parent, left, and right nodes) and a byte value to indicate the level (height of a corresponding node in the [2-3 tree](https://en.wikipedia.org/wiki/2%E2%80%933_tree)) of the current node. Embedding the `aatree_node_t` struct makes an intrusive container. The same object can be made to belong to an arbitrary number of different containers.

See "Balanced Search Trees Made Simple" by Arne Andersson,
http://user.it.uu.se/~arnea/ps/simp.pdf
