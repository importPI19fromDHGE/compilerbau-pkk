//
// Created by pkk on 29.03.22.
//

/**
 * Every non-terminal is a function returning a treenode_t-Pointer.
 * If nothing has matched, it will return NULL.
 */

#ifndef COMPILERBAU_PKK_PARSER_H
#define COMPILERBAU_PKK_PARSER_H

#ifndef COMPILERBAU_PKK_DYNAMIC_ARRAY_H
#include "dynamic_array.h"
#endif

// todo: remove probably (not globally required)
extern treenode_t *syntax_tree;

extern Array *token_stream;

treenode_t *program();
treenode_t *pathdef();
treenode_t *calcdef();
treenode_t *params();

/// checks on token with type "tok_bofeof" (program begin/end)
void program_begin_end();

/// get the current token in relation to token_index
/// \returns current token
const token_t *get_token();

/// inserts son_node into parent_node if 'not NULL
/// \param parent_node to insert son_node in
/// \param son_node which gets inserted
void add_son_node(treenode_t *parent_node, treenode_t *son_node);

/// creates new tree node with default initialized values
/// \returns the created tree node pointer
treenode_t *new_tree_node();


treenode_t *statements();

treenode_t *name(); // == var or name idk
treenode_t *var();
treenode_t *name_var_any();
treenode_t *determine_name_var();

treenode_t *color();
treenode_t *cond();
treenode_t *cond_s();
treenode_t *expr();
treenode_t *term();
treenode_t *faktor();
treenode_t *operand();
treenode_t *args();
treenode_t *statement();
treenode_t *draw_cmd();
treenode_t *calc_cmd();
treenode_t *if_cmd();
treenode_t *times_cmd();
treenode_t *cnt_cmd();
treenode_t *while_cmd();
treenode_t *repeat();

#endif