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

#ifndef _STRING_H
#include <string.h>
#endif

#define FNPTRS 8

/// should only increment after successfully processed.
int token_index = 0;
const token_t *capture_error_token = NULL;

// maybe look at first token and choose or skip cmd (func for each array entry)
treenode_t* (*cmd_ptrs[FNPTRS])(void) = { // this is where the fun* begins // now THIS is peak C programming
        cmd_mark,   // jump, walk, mark
        cmd_draw,   // walk, jump, turn, color, clear, stop, finish, path
        cmd_calc,   // store, add, sub, mul, div
        cmd_if,     // if
        cmd_do,     // do
        cmd_counter,// counter
        cmd_while,  // while
        cmd_repeat  // repeat
};

treenode_t *program() {
    treenode_t *node = NULL;

    assert(get_token(true)->type == tok_bofeof);
    while(get_token()->type != keyw_begin) {
        pathdef();
        calcdef();
    }

    // begin => (type == keyw_begin)
    assert_token(get_token(true)->type == keyw_begin, "begin keyword missing!");
    // fill statements of main treenode
    node = statements();
    assert_token(node != NULL, "missing statements");

    // end => (type == keyw_end)
    assert_token(get_token(true)->type == keyw_end, "end keyword missing!");
    assert(get_token(true)->type == tok_bofeof);

    assert(token_index == token_stream.used);
    return node;
}

void pathdef() {
    if (get_token()->type != keyw_path) {
        return;
    }
    token_index++;

    nameentry_t *func_target = name(false);
    assert_token(func_target != NULL, "Missing path name for path definition");
    printf("[debug] pathdef fct {%s} init for nametab_index {%d}\n",
           func_target->name, token_stream.array[token_index-1].data.name_tab_index
    );

    // function for the node in the syntaxtree
    funcdef_t *func = malloc(sizeof (funcdef_t));
    func->params[0] = NULL;
    func->params[1] = NULL;
    func->ret = NULL; // cause pathdef -> no return to fill (see funcdef_t)

    // init target
    func_target->type = name_path;
    func_target->d.func = func;

    // fill params when parentheses occur
    if (get_token()->type == oper_lpar) {
        token_index++;
        fill_params(func);
        assert_token(get_token(true)->type == oper_rpar, "pathdef: Missing closing parenthesis");
    }

    // fill body with statements
    func->body = NULL;
    func->body = statements();
    assert_token(func->body != NULL, "Missing statements for path definition");
    assert_token(get_token(true)->type == keyw_endpath, "Missing endpath keyword!");
}

void calcdef() {
    if (get_token()->type != keyw_calculation) {
        return;
    }
    token_index++;

    nameentry_t *func_target = name(false);
    assert_token(func_target != NULL, "Missing name for calc definition");
    printf("[debug] calcdef fct {%s} init for nametab_index {%d}\n",
           func_target->name, token_stream.array[token_index-1].data.name_tab_index
    );

    // function for the node in the syntaxtree
    funcdef_t *func = malloc(sizeof (funcdef_t));

    // init target
    func_target->type = name_calc;
    func_target->d.func = func;

    // fill params
    assert_token(get_token(true)->type == oper_lpar, "Missing opening parenthesis");
    fill_params(func);
    assert_token(get_token(true)->type == oper_rpar, "calcdef: Missing closing parenthesis");

    // fill body with statements | can be null
    func->body = NULL;
    func->body = statements(); // no check needed
    assert_token(get_token(true)->type == keyw_returns, "returns keyword is missing");

    // fill return with expression
    func->ret = expr();
    assert_token(func->ret != NULL, "missing expression");
    assert_token(get_token(true)->type == keyw_endcalc, "missing endcalc keyword");
}


nameentry_t *name(bool is_var) {
    nameentry_t *nameentry = &(name_tab[get_token()->data.name_tab_index]);
    const char *name = nameentry->name; // variable names for the mentally deranged (╯°□°）╯︵ ┻━┻

    switch (name[0]) {
        case '@':
            if (!is_var) {
                goto bad_code1; // i hate it // höhö LG Max (ノ°益°)ノ
            }
            break;
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '_':
            break; // check first char on all allowed chars - in this case, leave switch
        default:
        bad_code1:
            printf("Invalid starting character for name '%c'", name[0]);
            parser_error(NULL);
            break;
    }

    // check remaining chars on allowed chars
    for (int i = 1; name[i] != '\0'; i++) {
        switch (name[i]) {
            case 'a' ... 'z':
            case 'A' ... 'Z':
            case '0' ... '9':
            case '_':
                break;
            default:
                printf("Invalid character in name '%c'", name[i]);
                parser_error(NULL);
                break;
        }
    }

    token_index++;

    return nameentry;
}

nameentry_t *var() {
    switch (get_token()->type) {
        case name_any:
        case name_pvar_ro:
        case name_pvar_rw:
            break;
        default:
            return NULL;
    }

    nameentry_t *entry = name(true);
    if (entry->type == name_any) {
        entry->type = name_var;
    }
//    entry->type = name_var;

    return entry;
}

treenode_t *statement() {
    treenode_t *statement_to_add = NULL;

    for (int i = 0; i < FNPTRS; i++) {
        statement_to_add = cmd_ptrs[i]();
        if (statement_to_add != NULL) {
            break;
        }
    }

    return statement_to_add;
}

treenode_t* statements() {
    // statement() legt Speicher für statement-Knoten an, diese Funktion verknüpft das zu einer EVL in parent
    treenode_t *statement_tree = NULL; // top head
    treenode_t *active_statement = NULL; // current head of subtree
    do {
        if (statement_tree == NULL) {
            statement_tree = statement();
            active_statement = statement_tree;
        } else {
            active_statement->next = statement();
            active_statement = active_statement->next;
        }
    } while (active_statement != NULL);

    return statement_tree;
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

treenode_t *color() {
    const token_t *t;
    treenode_t *node = new_tree_node();
    node->type = keyw_color;

    for (int i = 0; i < 3; i++) {
        t = get_token();

        add_son_node(node, expr());
        
        if (i != 2) { // not last iteration
            assert_token(get_token(true)->type == oper_sep, "Missing comma for color value");
        }
    }

    return node;
}

void fill_args(treenode_t *parent_node) {
    bool has_added_token;
    type_t type;
    do {
        has_added_token = add_son_node(parent_node, expr());
        type = get_token()->type;
        if (parent_node != NULL && type == oper_sep) {
            token_index++;
            assert_token(has_added_token, "Missing expression after comma");
        }
    } while (type == oper_sep);
}

treenode_t *cond() {
    treenode_t *node = new_tree_node();
    treenode_t *first_cond;
    if (get_token()->type == oper_lpar) {
        token_index++;
        first_cond = cond_s();
        assert_token(first_cond, "missing condition");
        assert_token(get_token(true)->type == oper_rpar, "error in condition: expected expression");
    } else {
        first_cond = cond_s();
        assert_token(first_cond, "missing condition");
    }

    type_t multi_cond_type = get_token()->type;
    switch (multi_cond_type) {
        case keyw_not:
        case keyw_and:
        case keyw_or:
            node->type = multi_cond_type;
            token_index++;
            break;
        default:
            free(node);
            return first_cond;
    }
    treenode_t *second_cond = cond();
    assert_token(second_cond, "missing second condition after multi condition");
    add_son_node(node, first_cond);
    add_son_node(node, second_cond);

    return node;
}

treenode_t *cond_s() {
    treenode_t *node = new_tree_node();
    add_son_node(node,expr());
    const token_t *token = get_token(true);

    switch (token->type) {
        case oper_less:
        case oper_grtr:
        case oper_lequ:
        case oper_gequ:
        case oper_equ:
        case oper_nequ:
            node->type = token->type;
            break;
        default:
            parser_error("missing comparing operator");
    }

    add_son_node(node, expr());
    return node;
}

treenode_t *expr() { // fixme: nodes werden nicht weiter gegeben und somit entsteht ein tiefer baum
    treenode_t *node = new_tree_node();
    treenode_t *first_term = term();
    // token_index got incremented in term()

    assert_token(first_term, "error in expression: expected a term");

    type_t operator = get_token()->type;
    if (operator != oper_sub && operator != oper_add) {
        free(node);
        return first_term;
    }

    token_index++;
    // second term is always recursion to read long expressions
    treenode_t *second_term = expr();
    assert_token(second_term, "missing second term of expression");
    node->type = operator;
    add_son_node(node, first_term);
    add_son_node(node, second_term);

    return node;
}

treenode_t *term() {
    treenode_t *node = new_tree_node();
    treenode_t *first_factor = factor();

    assert_token(first_factor, "Missing first factor of term");
    type_t operator = get_token()->type;

    if (operator != oper_mul && operator != oper_div) {
        free(node);
        return first_factor;
    }
    token_index++;
    // second factor is always recursion to read long expressions
    treenode_t *second_factor = term();
    assert_token(second_factor, "Missing second factor of term");
    node->type = operator;
    add_son_node(node, first_factor);
    add_son_node(node, second_factor);

    // todo: perhaps recursion is needed

    return node;
}

treenode_t *factor() {
    treenode_t *node = new_tree_node(); // so position is right
    treenode_t *first_operand = operand();
    // token_index got incremented in operand()
    assert_token(first_operand, "factor: missing operand");
    // nicht vorhanden => keine node
    type_t operator = get_token()->type;
    if (operator != oper_pow) {
        free(node);
        return first_operand;
    }
    token_index++;
    // second operand is always recursion to read long expressions
    treenode_t *second_operand = factor();
    // token_index got incremented in factor()
    assert_token(second_operand, "factor: missing operand after ^");
    node->type = oper_pow;
    add_son_node(node, first_operand);
    add_son_node(node, second_operand);

    return node;
}

treenode_t *operand() {
    treenode_t *node = new_tree_node(); // head
    treenode_t *active_node = node;

    if (get_token()->type == oper_neg || get_token()->type == oper_sub) {
        token_index++;
        active_node->type = oper_neg;
        active_node = new_tree_node();
        add_son_node(node, active_node);
    }

    const token_t *token = get_token(true);
    switch (token->type) {
        // ("sqrt" | "sin" | "cos" | "tan") "(" EXPR ")"
        case name_math_sqrt:
        case name_math_sin:
        case name_math_cos:
        case name_math_tan:
            active_node->type = oper_lpar;
            active_node->d.p_name = &(name_tab[token->data.name_tab_index]);

            assert_token(get_token(true)->type == oper_lpar, "missing left bracket");
            assert_token(add_son_node(active_node, expr()), "Missing expression");
            assert_token(get_token(true)->type == oper_rpar, "missing right bracket");
            break;
        case name_pvar_ro:
        case name_pvar_rw: // fixme
            active_node->type = name_any; // token->type; // ?
            active_node->d.p_name = &(name_tab[token->data.name_tab_index]);
            break;
        // "rand" "(" EXPR "," EXPR ")"
        case name_math_rand:
            active_node->type = oper_lpar;
            active_node->d.p_name = &(name_tab[token->data.name_tab_index]);

            assert_token(get_token(true)->type == oper_lpar, "missing left bracket");

            assert_token(add_son_node(active_node, expr()), "Missing expression");
            assert_token(get_token(true)->type == oper_sep, "Missing comma");
            assert_token(add_son_node(active_node, expr()), "Missing expression after comma");

            assert_token(get_token(true)->type == oper_rpar, "missing right bracket");
            break;
        // "(" EXPR ")" | "|" EXPR "|"
        case oper_abs:
        case oper_lpar:
            memcpy(active_node, expr(), sizeof(treenode_t));

            assert_token(active_node, "Missing expression");
            if (token->type == oper_abs) {
                assert_token(get_token(true)->type == oper_abs, "missing right absolut");
            } else {
                assert_token(get_token(true)->type == oper_rpar, "missing right bracket");
            }
            break;
        // ZIFFER {ZIFFER} ["." {ZIFFER}]
        // done by lexer => copy value of token
        case oper_const:
            active_node->type = oper_const;
            active_node->d.val = token->data.val;
            break;
        // VAR | NAME "(" ARGS ")"
        case name_any:
            active_node->type = name_any;
            if (get_token()->type == oper_lpar) {
                active_node->type = oper_lpar;
                token_index--;
                active_node->d.p_name = name(false);
                // token_index got incremented in name()
                assert_token(get_token(true)->type == oper_lpar, "missing left bracket");
                fill_args(active_node);
                assert_token(get_token(true)->type == oper_rpar, "missing left bracket");
            } else {
                token_index--;
                active_node->d.p_name = var();
                assert_token(active_node->d.p_name, "operand: missing var");
            }
            break;
        default:
            token_index--;
            return NULL;
    }
    return node;
}

//
// Statement Commands
//

treenode_t *cmd_draw() {
    treenode_t *node = new_tree_node();
    node->type = get_token(true)->type;

    switch (node->type) {
        case keyw_walk:
        case keyw_jump:
            switch (get_token()->type) {
                case keyw_back:
                    node->d.walk = get_token(true)->type;
                    break;
                case keyw_home:
                    node->d.walk = get_token(true)->type;
                    return node;
                default:
                    node->d.walk = keyw_walk;
                    break;
            }
            break;
        case keyw_turn: // todo: keyw_turn kommt in der turtle-eval gar nicht vor?
            switch (get_token()->type) {
                case keyw_left:
                case keyw_right:
                    node->type = get_token(true)->type;
                    break;
                default:
                    node->type = keyw_right;
                    break;
            }
            break;
        case keyw_direction:
            break;
        case keyw_color:
            assert_token(node = color(), "missing value for color");
            // break;
            return node;
        // only check for keyword
        case keyw_clear:
        case keyw_stop:
        case keyw_finish:
            //break;
            return node;
        case keyw_path:
            assert_token(node->d.p_name = name(false), "missing name for path");
            if (get_token()->type == oper_lpar) {
                token_index++;
                fill_args(node);
                assert_token(get_token(true)->type == oper_rpar, "draw: missing closing parenthesis");
            }
            //break;
            return node;
        default:
            token_index--;
            free(node);
            return NULL;
    }

    assert_token(add_son_node(node, expr()), "missing expression");

    return node;
}

treenode_t *cmd_mark() {
    int last_token_index = token_index;
    type_t type = get_token()->type;
    switch (type) {
        case keyw_walk:
        case keyw_jump:
        case keyw_mark:
            break;
        default:
            return NULL;
    }

    treenode_t *node = new_tree_node();
    node->type = type;
    token_index++;

    if (type != keyw_mark) {
        if (get_token(true)->type == keyw_mark) {
            node->d.walk = keyw_mark;
        } else {
            token_index = last_token_index;
            return NULL;
        }
    }

    return node;
}

treenode_t *cmd_calc() {
    treenode_t *node = new_tree_node();
    node->type = get_token(true)->type;

    switch (node->type) {
        case keyw_store:
        case keyw_add:
        case keyw_sub:
            assert_token(add_son_node(node, expr()), "missing expression");
            switch (node->type) {
                case keyw_store:
                    assert_token(get_token(true)->type == keyw_in, "missing 'in' keyword");
                    break;
                case keyw_add:
                    assert_token(get_token(true)->type == keyw_to, "missing 'to' keyword");
                    break;
                case keyw_sub:
                    assert_token(get_token(true)->type == keyw_from, "missing 'from' keyword");
                    break;
                default:
                    assert(false);
            }
            assert_token(node->d.p_name = var(), "missing variable");
            break;
        case keyw_mul:
        case keyw_div:
            assert_token(node->d.p_name = var(), "missing variable");
            assert_token(get_token(true)->type == keyw_by, "missing 'by' keyword");
            assert_token(add_son_node(node, expr()), "missing expression");
            break;
        default:
            token_index--;
            free(node);
            return NULL;
    }

    return node;
}


treenode_t *cmd_if() {
    if (get_token()->type != keyw_if) {
        return NULL;
    }
    token_index++;

    treenode_t *node = new_tree_node();
    node->type = keyw_if;

    assert_token(add_son_node(node, cond()), "missing condition");
    assert_token(get_token(true)->type == keyw_then, "missing then keyword");

    // fill then statements -> son[1]
    node->son[1] = NULL;
    node->son[1] = statements();

    if (get_token()->type == keyw_else) {
        token_index++;
        // fill else statements -> son[2]
        node->son[2] = NULL;
        node->son[2] = statements();
    }

    assert_token(get_token(true)->type == keyw_endif, "missing endif keyword");

    return node;
}

treenode_t *cmd_do() {
    if (get_token()->type != keyw_do) {
        return NULL;
    }
    token_index++;

    treenode_t *node = new_tree_node();
    node->type = keyw_do;

    assert_token(add_son_node(node, expr()), "missing expression for times cmd");
    assert_token(get_token(true)->type == keyw_times, "missing times keyword");

    // fill statements into son 1
    node->son[1] = NULL;
    node->son[1] = statements();
    assert_token(get_token(true)->type == keyw_done, "missing done keyword");

    return node;
}

treenode_t *cmd_counter() {
    if (get_token()->type != keyw_counter) {
        return NULL;
    }
    token_index++;

    treenode_t *node = new_tree_node();
    node->type = keyw_counter;

    node->d.p_name = var();

    // holds VAR (see BNF) according to incremented token_index in node
    assert_token(node->d.p_name, "missing counter variable");
    // token_index got incremented in name()
    assert_token(get_token(true)->type == keyw_from, "Syntax error in counter command");
    // get_token(true) increments token_index
    // syntax ok, next token please

    treenode_t *start_val = expr();
    // token_index got incremented in expr()
    assert_token(start_val, "counter: missing start value");
    node->son[0] = start_val;

    type_t cnt_type = get_token()->type;
    switch (cnt_type) {
        case keyw_to:
        case keyw_downto:
            token_index++;
            break;
        default:
            parser_error("syntax error in counter: expected 'to' or 'downto', got something else");
    }

    treenode_t *end_val = expr();
    // token_index got incremented in expr()
    assert_token(end_val, "counter: missing end value");

    node->son[1] = NULL;
    node->son[2] = NULL;

    if (cnt_type == keyw_downto) {
        node->son[2] = end_val;
    } else {
        node->son[1] = end_val;
    }

    if (get_token()->type == keyw_step) {
        token_index++;
        treenode_t *step_val = expr();
        // token_index got incremented in expr()
        assert_token(step_val, "counter: missing step value");
        node->son[3] = step_val;
    }
    assert_token(get_token(true)->type == keyw_do, "syntax error in counter: expected \"do\"");
    // fill statements into son 4
    node->son[4] = NULL;
    node->son[4] = statements();
    assert_token(get_token(true)->type == keyw_done, "Missing done keyboard");

    return node;
}

treenode_t *cmd_while() {
    if (get_token()->type != keyw_while) {
        return NULL;
    }
    token_index++;

    treenode_t *node = new_tree_node();
    node->type = keyw_while;

    assert_token(add_son_node(node, cond()), "missing condition for while cmd");
    assert_token(get_token(true)->type == keyw_do, "missing do keyword");

    // fill statements into son 1
    node->son[1] = NULL;
    node->son[1] = statements();
    assert_token(get_token(true)->type == keyw_done, "missing done keyword");

    return node;
}

treenode_t *cmd_repeat() {
    if (get_token()->type != keyw_repeat) {
        return NULL;
    }
    token_index++;

    treenode_t *node = new_tree_node();
    node->type = keyw_repeat;

    // fill statements into son 1
    node->son[1] = NULL;
    node->son[1] = statements();
    assert_token(get_token(true)->type == keyw_until, "missing until keyword");

    assert_token(add_son_node(node, cond()), "missing condition for repeat cmd");

    return node;
}

// HELPER FUNCTIONS BELOW

void parser_error(const char *msg) {
    if (msg != NULL) {
        printf("%s\n", msg);
    }
    if (capture_error_token == NULL) {
        capture_error_token = get_token();
    }
    printf("Error at: %d:%d\n", capture_error_token->pos.line, capture_error_token->pos.col);
    exit(1);
}

const token_t *def_get_token(get_token_args_t args) {
    bool increment_index = args.increment_index ? args.increment_index : false; // default value = false
    capture_error_token = &(token_stream.array[ token_index ]);
    return &(token_stream.array[ increment_index ? token_index++ : token_index ]);
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
        capture_error_token = NULL;
        return;
    }
    parser_error(msg);
}
