//
// Created by pkk on 29.03.22.
//
#ifndef _MALLOC_H
#include <malloc.h>
#endif

#ifndef _ASSERT_H
#include <assert.h>
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#include "parser.h"

/// should only increment after successfully processed.
int token_index = 0;

void parser_error(const char *msg) {
    if (msg != NULL) {
        printf("%s\n", msg);
    }
    printf("Error at: %d:%d\n", get_token()->pos.line, get_token()->pos.col);
    exit(1);
}

treenode_t *program() {
    treenode_t *node = new_tree_node();

    program_begin_end();
    while(get_token()->type != keyw_begin) {
        add_son_node(node, pathdef());
        add_son_node(node, calcdef());
    }
    // begin => (type == keyw_begin)
    if (get_token()->type == keyw_begin) {
        token_index++;
    } else {
        parser_error("begin keyword missing!");
    }
    statements();

    // end => (type == keyw_end)
    if (get_token()->type == keyw_end) {
        token_index++;
    } else {
        parser_error("end keyword missing!");
    }

    program_begin_end();
    assert(token_index == token_stream.used);

    return node;
}

// tok_bofeof
void program_begin_end() {
    assert(get_token()->type == tok_bofeof);
    token_index++;
}

treenode_t *pathdef() {
    treenode_t *node = new_tree_node();

    if (get_token()->type != keyw_path) {
        return NULL;
    }
    token_index++;
    if (!add_son_node(node, name())) {
        parser_error("Syntax error at path definition (could not find path name)"); // TODO: SPRICH
    }
    if (get_token()->type == oper_lpar){
        token_index++;
        add_son_node(node, params());
        if (get_token()->type != oper_rpar) {
            parser_error("Missing closing parenthesis");
        }
        token_index++;
    }
    if (!add_son_node(node, statements())) {
        parser_error("Missing statements for path definition");
    }
    if (get_token()->type != keyw_endpath) {
        parser_error("Missing endpath keyword!");
    }
    token_index++;

    return node;
}

treenode_t *calcdef() {
    treenode_t *node = new_tree_node();

    if (get_token()->type != keyw_calculation) {
        return NULL;
    }
    token_index++;
    assert_token(add_son_node(node, name()), "Missing path name for calc definition");
    assert_token(get_token()->type == oper_lpar, "Missing opening parenthesis");

    token_index++;
    add_son_node(node, params());
    assert_token(get_token()->type == oper_rpar, "Missing closing parenthesis");

    token_index++;
    add_son_node(node, statements());
    assert_token(get_token()->type == keyw_returns, "returns keyword is missing");

    token_index++;
    assert_token(add_son_node(node, expr()), "missing expression");
    assert_token(get_token()->type == keyw_endcalc, "missing endcalc keyword");

    token_index++;

    return node;
}

// HELPER FUNCTIONS BELOW

// big todo: maybe increment token_index here... depending on duplicate code
const token_t *get_token() {
    return &(token_stream.array[token_index]);
}

bool add_son_node(treenode_t *parent_node, treenode_t *son_node) {
    if (son_node == NULL) {
        return false;
    }
    parent_node->son[parent_node->son_len] = son_node;
    parent_node->son_len++;

    return true;
}

treenode_t *new_tree_node() {
    treenode_t *node = malloc(sizeof(treenode_t));
    node->son_len = 0;
    node->next = NULL;
    node->pos = get_token()->pos;

    return node;
}

void assert_token(bool expression, const char* msg) {
    if (expression) {
        return;
    }
    parser_error(msg);
}