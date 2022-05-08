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
        cmd_draw,
        cmd_mark,
        cmd_calc,
        cmd_if,
        cmd_do,
        cmd_counter,
        cmd_while,
        cmd_repeat
};

treenode_t *program() {
    treenode_t *node = new_tree_node();

    assert(get_token(true)->type == tok_bofeof);
    while(get_token()->type != keyw_begin) {
        add_son_node(node, pathdef());
        add_son_node(node, calcdef());
    }
    // begin => (type == keyw_begin)
    assert_token(get_token(true)->type == keyw_begin, "begin keyword missing!");
    fill_statements(node);

    // end => (type == keyw_end)
    assert_token(get_token(true)->type == keyw_end, "end keyword missing!");
    assert(get_token(true)->type == tok_bofeof);// todo: increment yes/no?

    assert(token_index == token_stream.used);
    return node;
}

treenode_t *pathdef() {
    treenode_t *node = new_tree_node();
    node->type = name_path;
    // function for the node in the syntaxtree
    funcdef_t *func = malloc(sizeof (funcdef_t));
    func->ret = NULL; // cause pathdef -> no return to fill (see funcdef_t)

    if (get_token()->type != keyw_path) {
        return NULL;
    }
    token_index++;

    nameentry_t *func_entry = name(false);
    assert_token(func_entry != NULL, "Missing path name for path definition");

    // assign func and func_entry
    func_entry->type = name_calc;
    func_entry->d.func = func;
    node->d.p_name = func_entry;

    // fill params when parentheses occur
    if (get_token()->type == oper_lpar) {
        token_index++;
        fill_params(func);
        assert_token(get_token(true)->type == oper_rpar, "Missing closing parenthesis");
    }

    // fill body with statements
    treenode_t *body = new_tree_node();
    func->body = body;
    fill_statements(body);
    assert_token(body != NULL, "Missing statements for path definition");
    assert_token(get_token(true)->type == keyw_endpath, "Missing endpath keyword!");

    return node;
}

treenode_t *calcdef() {
    treenode_t *node = new_tree_node();
    node->type = name_calc;
    // function for the node in the syntaxtree
    funcdef_t *func = malloc(sizeof (funcdef_t));

    if (get_token()->type != keyw_calculation) {
        return NULL;
    }
    token_index++;

    nameentry_t *func_entry = name(false);
    assert_token(func_entry != NULL, "Missing path name for path definition");

    // assign func and func_entry
    func_entry->type = name_calc;
    func_entry->d.func = func;
    node->d.p_name = func_entry;

    // fill params
    assert_token(get_token(true)->type == oper_lpar, "Missing opening parenthesis");
    fill_params(func);
    assert_token(get_token(true)->type == oper_rpar, "Missing closing parenthesis");

    // fill body with statements | can be null -> todo: don't forgor free if no statement inserted
    treenode_t *body = new_tree_node();
    fill_statements(body);
    func->body = body;
    assert_token(get_token(true)->type == keyw_returns, "returns keyword is missing");

    // fill return with expression
    treenode_t *ret = new_tree_node();
    assert_token(add_son_node(ret, expr()), "missing expression");
    func->ret = ret;
    assert_token(get_token(true)->type == keyw_endcalc, "missing endcalc keyword");

    return node;
}


nameentry_t *name(bool is_var) {
    nameentry_t *nameentry = &(name_tab[get_token()->data.name_tab_index]);
    const char *name = nameentry->name; // variable names for the mentally deranged (╯°□°）╯︵ ┻━┻

    switch (name[0]) {
        case '@':
            if (!is_var) {
                goto bad_code1; // i hate it // höhö LG Max (ノ°益°)ノ // todo: rework plz
            }
            break;
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '_':
            break; // check all allowed chars - in this case, leave switch
        default:
        bad_code1:
            printf("Invalid starting character for name '%c'", name[0]);
            parser_error(NULL);
            break;
    }

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
    if (get_token()->type != name_any) {
        return NULL;
    }

    nameentry_t *entry = name(true);
    entry->type = name_var;

    return entry;
}

treenode_t *statement() {
    // fixme: oh boi... yes... this... notLikeThisCat
    treenode_t *statement_to_add = NULL;

    // preferably a switch case... but c shenanigans ¯\_(ツ)_/¯
    // ┌∩┐(◣_◢)┌∩┐
    for (int i = 0; i < FNPTRS; i++) {
        statement_to_add = cmd_ptrs[i]();
        if (statement_to_add != NULL) {
            break;
        }
    }

    return statement_to_add;
}

void fill_statements(treenode_t *parent) {
    // statement() legt Speicher für statement-Knoten an, diese Funktion verknüpft das zu einer EVL in parent
    treenode_t *st;
    treenode_t *target; // = parent->son[parent->son_len];
    bool statements_found = false;

    while ((st = statement()) != NULL) {

        statements_found = true;
        target = (treenode_t*) malloc(sizeof(treenode_t));
        memcpy(target, st, sizeof(treenode_t));
//        *target = *st; // zpm: i have some worries here, to be tested
        parent->son[parent->son_len] = target;
        free(st); // Pointer unneeded - at least one mem leak less
        target = target->next; // classic EVL - move pointer to next statement
    }
    if (statements_found) { // if statements found, increment son length once
        parent->son_len++;
    } else {
        parser_error("missing at least one valid statement");
    }
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

        if (t->type != oper_const ||
            t->data.val < 0 || t->data.val > 100) {
            parser_error("Invalid color value");
        }

        treenode_t *color = new_tree_node();
        color->d.val = t->data.val;
        add_son_node(node, color);
        
        token_index++;
        
        if (i != 2) { // not last iteration
            assert_token(get_token(true)->type != oper_sep, "Missing comma for color value");
        }
    }

    return node;
}

void fill_args(treenode_t *parent_node) {
    bool has_added_token;
    do {
        has_added_token = add_son_node(parent_node, expr());
        if (parent_node != NULL && parent_node->son_len > 0) {
            assert_token(has_added_token, "Missing expression after comma");
        }
    } while (get_token(true)->type == oper_sep);
}

treenode_t *cond() {
    treenode_t *node = new_tree_node();
    if (get_token()->type == oper_lpar) {
        token_index++;
        node = cond_s(node);
        assert_token(get_token(true)->type == oper_rpar, "error in condition: expected expression");
    } else {
        node = cond_s(node);
    }

    return node;
}

treenode_t *cond_s(treenode_t *node) {
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

    switch (get_token()->type) {
        case keyw_not:
        case keyw_and:
        case keyw_or:
            node->son[1]->type = get_token(true)->type;
            cond_s(node->son[1]); // todo: check if this works
            break;
        default:
            break;
    }

    return node;
}

treenode_t *expr() { //todo fixme
    treenode_t *node = new_tree_node();
    assert_token(add_son_node(node, term()), "error in expression: expected a term");
    // token_index got incremented in term()
    type_t type = get_token()->type;
    // token_index got incremented in get_token(true)
    while ((type == oper_add) || (type == oper_sub)) {
        token_index++;
        assert_token(add_son_node(node, term()), "error in expression: expected a term");
        // token_index got incremented in term()
        type = get_token()->type;
        // token_index got incremented in get_token(true)
    }

    return node;
}

treenode_t *term() {
    treenode_t *node = new_tree_node();
    assert_token(add_son_node(node, factor()), "error in expression: expected a factor");
    // token_index got incremented in term()
    type_t type = get_token()->type;
    // token_index got incremented in get_token(true)
    while ((type == oper_mul) || (type == oper_div)) {
        token_index++;
        assert_token(add_son_node(node, factor()), "error in expression: expected a factor");
        // token_index got incremented in term()
        type = get_token()->type;
        // token_index got incremented in get_token(true)
    }

    return node;
}

treenode_t *factor() {
    treenode_t *node = new_tree_node();
    node->type = oper_pow;
    assert_token(add_son_node(node, operand()), "missing operand");

    if (get_token()->type == oper_pow) {
        token_index++;
        assert_token(add_son_node(node, factor()), "missing operand after ^");
    }

    return node;
}

treenode_t *operand() {
    treenode_t *node = new_tree_node();
    treenode_t *active_node = node;

    if (get_token()->type == oper_neg) {
        token_index++;
        active_node->type = oper_neg;
        active_node = new_tree_node();
    }

    const token_t *token = get_token(true);
    switch (token->type) {
        // ("sqrt" | "sin" | "cos" | "tan") "(" EXPR ")"
        case name_math_sqrt:
        case name_math_sin:
        case name_math_cos:
        case name_math_tan: {
            treenode_t *math_node = new_tree_node();
            math_node->type = token->type; // theoretically not required by the turtle-eval
            math_node->d.p_name = &(name_tab[token->data.name_tab_index]);
            add_son_node(active_node, math_node);

            assert_token(get_token(true)->type == oper_lpar, "missing left bracket");
            assert_token(add_son_node(math_node, expr()), "Missing expression");
            assert_token(get_token(true)->type == oper_rpar, "missing right bracket");
            break;
        }
        // "rand" "(" EXPR "," EXPR ")"
        case name_math_rand: {
            treenode_t *rand_node = new_tree_node();
            rand_node->type = token->type;
            rand_node->d.p_name = &(name_tab[token->data.name_tab_index]);
            add_son_node(active_node, rand_node);

            assert_token(add_son_node(rand_node, expr()), "Missing expression");
            assert_token(get_token(true)->type == oper_sep, "Missing comma");
            assert_token(add_son_node(rand_node, expr()), "Missing expression after comma");
            break;
        }
        // "(" EXPR ")" | "|" EXPR "|"
        case oper_abs:
        case oper_lpar:
            assert_token(add_son_node(active_node, expr()), "Missing expression");
            if (token->type == oper_abs) {
                assert_token(get_token(true)->type == oper_abs, "missing right absolut");
            } else {
                assert_token(get_token(true)->type == oper_rpar, "missing right bracket");
            }
            break;
        // ZIFFER {ZIFFER} ["." {ZIFFER}]
        // done by lexer => copy value of token
        case oper_const: {
            treenode_t *const_node = new_tree_node();
            const_node->type = oper_const;
            const_node->d.val = token->data.val;
            break;
        }
        // VAR
        case name_any: {
            treenode_t *var_node = new_tree_node();
            var_node->d.p_name = var();
            add_son_node(active_node, var_node);
            break;
        }
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
                case keyw_home:
                    node->d.walk = get_token(true)->type;
                    break;
                default:
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
                    break;
            }
            break;
        case keyw_color:
            assert_token(node = color(), "missing value for color");
            break;
        // only check for keyword
        case keyw_direction:
        case keyw_clear:
        case keyw_stop:
        case keyw_finish:
            break;
        case keyw_path:
            assert_token(node->d.p_name = name(false), "missing name for path");
            if (get_token()->type == oper_lpar) {
                token_index++;
                fill_args(node);
                assert_token(get_token(true)->type == oper_rpar, "missing closing parenthesis");
            }
            break;
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
            assert_token(get_token(true)->type == keyw_by, "missing 'from' keyword");
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

    fill_statements(node);

    if (get_token()->type == keyw_else) {
        token_index++;
        fill_statements(node);
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
    assert_token(get_token(true)->type == keyw_times, "missing times keyword"); // todo fixme

    fill_statements(node);
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

    // holds VAR (see BNF) according to incremented token_index in node
    assert_token(node->d.p_name = name(true), "missing counter variable");
    // token_index got incremented in name()
    assert_token(get_token(true)->type == keyw_from, "Syntax error in counter command");
    // get_token(true) increments token_index
    // syntax ok, next token please
    assert_token(add_son_node(node, expr()), "missing expression in counter");
    // token_index got incremented in expr()
    type_t cnt_type = get_token()->type;
    switch (cnt_type) {
        case keyw_to:
        case keyw_downto:
            // todo: figure out if i have to do something here
            break;
        default:
            parser_error("syntax error in counter: expected 'to' or 'downto', got something else");
    }
    token_index++;
    assert_token(add_son_node(node, expr()), "missing expression in counter");
    // token_index got incremented in expr()
    if (get_token()->type == keyw_step) {
        assert_token(add_son_node(node, expr()), "missing expression for step in counter");
        // token_index got incremented in expr()
    }
    assert_token(get_token(true)->type == keyw_do, "syntax error in counter: expected \"do\"");
    token_index++;
    fill_statements(node);

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

    fill_statements(node);
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

    fill_statements(node);
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
