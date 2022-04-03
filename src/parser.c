//
// Created by pkk on 29.03.22.
//
#ifndef _MALLOC_H
#include <malloc.h>
#endif
#ifndef _ASSERT_H
#include <assert.h>
#endif
#include "parser.h"

/// should only increment after successfully processed
int token_index = 0;

treenode_t *program() {
    treenode_t *node = new_tree_node();

    program_begin_end();
    while(get_token()->type != keyw_begin) {
        add_son_node(node, pathdef());
        add_son_node(node, calcdef());
    }

    // begin => (type == keyw_begin)
    statements();
    // end => (type == keyw_end)

    program_begin_end();
    assert(token_index == token_stream->used);

    return node;
}

// tok_bofeof
void program_begin_end() {
    assert(get_token()->type == tok_bofeof);
    token_index++;
}

const token_t *get_token() {
    return &(token_stream->array[token_index]);
}

void add_son_node(treenode_t *parent_node, treenode_t *son_node) {
    if (son_node == NULL) {
        return;
    }
    parent_node->son[parent_node->son_len] = son_node;
    parent_node->son_len++;
}

treenode_t *new_tree_node() {
    treenode_t *node = malloc(sizeof(treenode_t));
    node->son_len = 0;
    node->next = NULL;
    node->pos = get_token()->pos;

    return node;
}