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

#define get_token(...) def_get_token((get_token_args_t){__VA_ARGS__})

/// stream of token created by lexer.
extern Array token_stream;

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
/// name or var of a function; increments token_index.
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
treenode_t* statements();

/// STATEMENT  ::= DRAW_CMD | MARK_CMD | CALC_CMD | IF_CMD | TIMES_CMD | CNT_CMD | WHILE_CMD | REPEAT_CMD.
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *statement();

///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *color();

/// ARGS ::= [ EXPR { "," EXPR } ]
void fill_args(treenode_t *parent_node);

///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *cond();

treenode_t *cond_s(treenode_t *node);

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

/// OPERAND ::= [ "-" ] ( "sqrt" | "sin" | "cos" | "tan" "(" EXPR ")" | "rand" "(" EXPR "," EXPR ")" | "(" EXPR ")" | "|" EXPR "|"  | ZIFFER {ZIFFER} ["." {ZIFFER}] | VAR )
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *operand();

///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *cmd_draw();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *cmd_mark();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *cmd_calc();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *cmd_if();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *cmd_do();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *cmd_counter();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *cmd_while();
///
/// \returns a treenode_t* matching to description or default (see parser.h)
treenode_t *cmd_repeat();


/// Print an error message related to the currently used token, depending on token_index or a set token.
/// \param msg A custom Error message. Set to NULL to print a generic / unkown error.
void parser_error(const char *msg);

typedef struct get_token_args {
    bool increment_index;
} get_token_args_t;

/// get the current token in relation to token_index.
/// \param args struct to impl a default value solution
/// \returns current token
const token_t *def_get_token(get_token_args_t args);

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

void assign_head_or_next(treenode_t **head, treenode_t **next_head, treenode_t* (*fptr)());

void fill_statements(treenode_t **node_to_fill);

#endif