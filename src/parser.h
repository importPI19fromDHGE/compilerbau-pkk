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

// todo: remove probably (not required globally)
extern treenode_t *syntax_tree;

/// stream of token created by lexer.
extern Array token_stream;


/// checks on token with type "tok_bofeof" (program begin/end).
void program_begin_end();

/// Print an error message related to the currently used token, depending on token_index.
/// \param msg A custom Error message. Set to NULL to print a generic / unkown error.
void parser_error(const char *msg);


/// PROGRAM ::= { PATHDEF | CALCDEF } "begin" STATEMENTS "end".
/// \returns the treenode_t* syntax tree required by evaluate in main
treenode_t *program();

/*
////////// returns a treenode_t* or NULL if no matching was found
*/

/// PATHDEF ::= "path" NAME [ "("[ PARAMS ]")" ] STATEMENTS "endpath"
/// definition of a path function.
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *pathdef();

/// CALCDEF ::= "calculation" NAME "("[ PARAMS ]")" [ STATEMENTS ] "returns" EXPR "endcalc"
/// definition of a calculation function.
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *calcdef();

/// NAME ::= ["@"] ( BUCHST | "_" ) { BUCHST | "_" | ZIFFER }.
/// name of a function.
/// \returns a nameentry_t* matching to description or default (see parser.h)
nameentry_t *name(bool is_var); // == var or name idk

/// VAR ::= NAME
/// name of a variable.
/// \returns a nameentry_t* matching to description or default (see parser.h)
nameentry_t *var();

/// PARAMS ::= [ VAR { "," VAR } ]
/// insert fill_params to a function.
void *fill_params(funcdef_t *func /**< func to insert */);

/// concatenates statement-nodes from statement() to an EVL in the parent-node.
/// STATEMENTS ::= STATEMENT { STATEMENT }.
void fill_statements(treenode_t *parent /**< The parent node where the statements will be linked*/);

/// STATEMENT  ::= DRAW_CMD | MARK_CMD | CALC_CMD | IF_CMD | TIMES_CMD | CNT_CMD | WHILE_CMD | REPEAT_CMD.
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *statement();

///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *color();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *cond();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *cond_s();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *expr();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *term();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *factor();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *operand();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *args();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *draw_cmd();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *mark_cmd();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *calc_cmd();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *if_cmd();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *times_cmd();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *cnt_cmd();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *while_cmd();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *repeat_cmd();


/// get the current token in relation to token_index.
/// \returns current token
const token_t *get_token();

/// inserts son_node into parent_node if 'not NULL.
/// \returns true if the provided node was not NULL and added, otherwise false
bool add_son_node(treenode_t *parent_node, ///< node to insert son_node in
                  treenode_t *son_node ///< node to insert
                  );

/// creates new tree node with default initialized values.
/// \returns the created tree node pointer
treenode_t *new_tree_node();

/// holds logic for error messages related to token operations.
void assert_token(bool expression, ///< Boolean for determining success of operation as returned by a helper function.
                  const char* msg ///< An Error message.
                  );

#endif