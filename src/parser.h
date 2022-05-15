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

typedef struct get_token_args {
    bool increment_index;
} get_token_args_t;

/// Dynamic Array of tokens created by lexer.
extern Array token_stream;

/// The wrapping program function.
/// PROGRAM ::= { PATHDEF | CALCDEF } "begin" STATEMENTS "end".
/// \returns the treenode_t* syntax tree required by evaluate in main
treenode_t *program();

/// Definition of a path function.
/// PATHDEF ::= "path" NAME [ "("[ PARAMS ]")" ] STATEMENTS "endpath".
void pathdef();

/// Definition of a calculation function.
/// CALCDEF ::= "calculation" NAME "("[ PARAMS ]")" [ STATEMENTS ] "returns" EXPR "endcalc".
void calcdef();

/// Fill Parameters for a function.
/// PARAMS ::= [ VAR { "," VAR } ].
/// \param func function to insert.
void *fill_params(funcdef_t *func);

/// Name or Variable of a function; increments token_index.
/// NAME ::= ["@"] ( BUCHST | "_" ) { BUCHST | "_" | ZIFFER }.
/// \param is_var is it a var or just a name?
/// \returns a nameentry_t* matching to description or NULL.
nameentry_t *name(bool is_var);

/// Name of a variable.
/// VAR ::= NAME.
/// \returns a nameentry_t* matching to description or NULL.
nameentry_t *var();

/// Build a color node consisting of R, G and B components.
/// COLOR ::= EXPR "," EXPR "," EXPR.
/// \returns a treenode_t* matching to description or NULL.
treenode_t *color();

/// Concatenates statement-nodes from statement() to an EVL in the parent-node.
/// STATEMENTS ::= STATEMENT { STATEMENT }.
/// \returns a treenode_t* matching to description or NULL.
treenode_t* statements();

/// Build a statement node.
/// STATEMENT ::= DRAW_CMD | MARK_CMD | CALC_CMD | IF_CMD | TIMES_CMD | CNT_CMD | WHILE_CMD | REPEAT_CMD.
/// \returns a treenode_t* matching to description or NULL.
treenode_t *statement();

/// Fill arguments of function calls.
/// ARGS ::= [ EXPR { "," EXPR } ].
void fill_args(treenode_t *parent_node);

/// Build a condition node consisting of one or multiple conditions.
/// ( "(" COND_S ")" | COND_S ) [ ( "not" | "and" | "or" ) COND ].
/// \returns a treenode_t* matching to description or NULL.
treenode_t *cond();

/// Build a single condition node.
/// EXPR ("<" | ">" | "<=" | ">=" | "=" | "<>") EXPR.
/// \returns a treenode_t* matching to description or NULL.
treenode_t *cond_s();

/// Build an expr node.
/// TERM [ ( "-" | "+" ) EXPR ].
/// \returns a treenode_t* matching to description or NULL.
treenode_t *expr();

/// Build a term node.
/// FAKTOR [ ( "*" | "/" ) TERM ].
/// \returns a treenode_t* matching to description or NULL.
treenode_t *term();

/// Build a recursive factor node.
/// OPERAND [ "^" FAKTOR ].
/// \returns a treenode_t* matching to description or NULL.
treenode_t *factor();

/// Build an operand node.
/// OPERAND ::= [ "-" ] ( "sqrt" | "sin" | "cos" | "tan" "(" EXPR ")" | "rand" "(" EXPR "," EXPR ")" | "(" EXPR ")" | "|" EXPR "|"  | ZIFFER {ZIFFER} ["." {ZIFFER}] | VAR ).
/// \returns a treenode_t* matching to description or NULL.
treenode_t *operand();

/// Build a drawing-related command node.
/// ((("walk" ["back"]) | ("jump" ["back"]) | ("turn" ["left" | "right"]) | "direction") EXPR) | ( ("walk" | "jump") "home" ) | ("color" COLOR) | "clear" | "stop" | "finish" | ("path" NAME ["(" [ ARGS ] ")"]).
/// \returns a treenode_t* matching to description or NULL.
treenode_t *cmd_draw();

/// Build a mark related command node.
/// ("mark") | ("walk mark") | ("jump mark").
/// \returns a treenode_t* matching to description or NULL.
treenode_t *cmd_mark();

/// Build a calculation node.
/// ("store" EXPR "in" VAR) | ("add" EXPR "to" VAR) | ("sub" EXPR "from" VAR) | ("mul" VAR "by" EXPR) | ("div" VAR "by" EXPR).
/// \returns a treenode_t* matching to description or NULL.
treenode_t *cmd_calc();

/// Build an if node.
/// "if" COND "then" STATEMENTS ["else" STATEMENTS] "endif".
/// \returns a treenode_t* matching to description or NULL.
treenode_t *cmd_if();

/// Build a do-times loop node.
/// "do" EXPR "times" STATEMENTS "done".
/// \returns a treenode_t* matching to description or NULL.
treenode_t *cmd_do();

/// Build a counter-loop node.
/// "counter" VAR "from" EXPR ("to" | "downto") EXPR ["step" EXPR] "do" STATEMENTS "done".
/// \returns a treenode_t* matching to description or NULL.
treenode_t *cmd_counter();

/// Build a while loop node.
/// "while" COND "do" STATEMENTS "done".
/// \returns a treenode_t* matching to description or NULL.
treenode_t *cmd_while();

/// Build a repeat-loop node.
/// "repeat" STATEMENTS "until" COND.
/// \returns a treenode_t* matching to description or NULL.
treenode_t *cmd_repeat();

/// Print an error message related to the currently used token, depending on token_index or a set token.
/// \param msg A custom Error message. Set to NULL to print a generic / unkown error.
void parser_error(const char *msg);

/// Get the current token in relation to token_index.
/// \param args struct to impl a default value solution.
/// \returns current token.
const token_t *def_get_token(get_token_args_t args);

/// Inserts son_node into parent_node if 'not NULL.
/// \param parent_node node to insert son_node in.
/// \param son_node node to insert.
/// \returns true if the provided node was not NULL and added, otherwise false.
bool add_son_node(treenode_t *parent_node, treenode_t *son_node);

/// Creates new tree node with default initialized values.
/// \returns the created tree node pointer.
treenode_t *new_tree_node();

/// Holds logic for error messages related to token operations.
/// \param expression Boolean for determining success of operation as returned by a helper function.
/// \param msg An Error message.
void assert_token(bool expression, const char* msg);

#endif