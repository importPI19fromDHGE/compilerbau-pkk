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

#ifndef _TURTLE_H
#include "turtle.h"
#endif

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
    node->type = name_path;

    if (get_token()->type != keyw_path) {
        return NULL;
    }
    token_index++;
    assert_token(add_son_node(node, name()), "Missing path name for path definition");

    if (get_token()->type == oper_lpar){
        token_index++;
        add_son_node(node, fill_params());
        assert_token(get_token()->type == oper_rpar, "Missing closing parenthesis");
        token_index++;
    }
    assert_token(add_son_node(node, statements()), "Missing statements for path definition");
    assert_token(get_token()->type == keyw_endpath, "Missing endpath keyword!");
    token_index++;

    return node;
}

treenode_t *calcdef() {
    // treenode for syntaxtree
    treenode_t *node = new_tree_node();
    node->type = name_calc;

    // function which gets called
    funcdef_t *func = malloc(sizeof (funcdef_t));
    // node in which the statements get inserted
    treenode_t *body = new_tree_node(); // can be null -> don't forgor free
    func->body = body; // body can also be NULL in calcdef

    if (get_token()->type != keyw_calculation) {
        return NULL;
    }
    token_index++;
    // todo: call error with "Missing path name for calc definition"
    nameentry_t *func_entry = name();

    assert_token(get_token()->type == oper_lpar, "Missing opening parenthesis");
    token_index++;
    fill_params(func);
    assert_token(get_token()->type == oper_rpar, "Missing closing parenthesis");
    token_index++;

    // node in which the return expression gets inserted
    treenode_t *ret = new_tree_node();
    func->ret = ret; // cant be null in calc def
    add_son_node(body, statements());
    assert_token(get_token()->type == keyw_returns, "returns keyword is missing");

    token_index++;
    assert_token(add_son_node(ret, expr()), "missing expression");
    assert_token(get_token()->type == keyw_endcalc, "missing endcalc keyword");

    token_index++;

    // set type of nameentry to satisfy kusche evaluation
    func_entry->type = name_calc;
    node->d = (nodedata_t) func_entry;

    return node;
}


nameentry_t *name() {
    // todo: check if name adheres to convention (and make sure it exists)

}

nameentry_t *var() {
    // todo: make call to name() instead
    if (get_token()->type != name_any) {
        return NULL;
    }

    nameentry_t *entry = malloc(sizeof(nameentry_t));
    entry->type = get_token()->type;
    entry->name = name_tab[get_token()->data.name_tab_index].name;

    token_index++;

    return entry;
}

void *fill_params(funcdef_t *func) {
//    nameentry_t *entry = malloc(sizeof(nameentry_t) * MAX_ARGS);
    int index = 0;

    /// name_any oper_sep name_any oper_sep name_any oper_rpar
    nameentry_t *v = var();
    func->params[index] = v;

    while(get_token()->type == oper_sep) {
        index++;
        token_index++;
        v = var();
        assert_token(v != NULL, "Missing variable after ");
        func->params[index] = v;
    }
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