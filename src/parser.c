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

#define FNPTRS 8

/// should only increment after successfully processed.
int token_index = 0;

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
    fill_statements(node);

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
        assert_token(get_token()->type == oper_rpar, "Missing closing parenthesis");
        token_index++;
    }

    // fill body with statements
    treenode_t *body = new_tree_node();
    func->body = body;
    fill_statements(body);
    assert_token(body != NULL, "Missing statements for path definition");
    assert_token(get_token()->type == keyw_endpath, "Missing endpath keyword!");
    token_index++;

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
    assert_token(get_token()->type == oper_lpar, "Missing opening parenthesis");
    token_index++;
    fill_params(func);
    assert_token(get_token()->type == oper_rpar, "Missing closing parenthesis");
    token_index++;

    // fill body with statements | can be null -> todo: don't forgor free if no statement inserted
    treenode_t *body = new_tree_node();
    fill_statements(body);
    func->body = body;
    assert_token(get_token()->type == keyw_returns, "returns keyword is missing");
    token_index++;

    // fill return with expression
    treenode_t *ret = new_tree_node();
    assert_token(add_son_node(ret, expr()), "missing expression");
    func->ret = ret;
    assert_token(get_token()->type == keyw_endcalc, "missing endcalc keyword");
    token_index++;

    return node;
}


nameentry_t *name(bool is_var) {
    const token_t *token = get_token();
    nameentry_t *nameentry = &(name_tab[token->data.name_tab_index]);
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
    treenode_t *target = parent->son[parent->son_len];
    bool statements_found = false;

    while ((st = statement()) != NULL) {

        statements_found = true;
        *target = *st; // zpm: i have some worries here, to be tested
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
            if (get_token()->type != oper_sep) {
                parser_error("Missing comma for color value");
            }
            token_index++;
        }
    }

    return node;
}

void fill_args(treenode_t *parent_node) {
    bool has_added_token;
    const token_t *token;
    do {
        has_added_token = add_son_node(parent_node, expr());
        if (parent_node != NULL && parent_node->son_len > 0) {
            assert_token(has_added_token, "Missing expression after comma");
        }
        token = get_token();
        token_index++;
    } while (token->type == oper_sep);
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
        token_index++; // todo: when moving token_index++ in get_token, add else with token_index--
        active_node->type = oper_neg;
        active_node = new_tree_node();
    }

    const token_t *token = get_token();
    token_index++;
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

            assert_token(get_token()->type == oper_lpar, "missing left bracket");
            token_index++;
            assert_token(add_son_node(math_node, expr()), "Missing expression");
            assert_token(get_token()->type == oper_rpar, "missing right bracket");
            token_index++;
            break;
        }
        // "rand" "(" EXPR "," EXPR ")"
        case name_math_rand: {
            treenode_t *rand_node = new_tree_node();
            rand_node->type = token->type;
            rand_node->d.p_name = &(name_tab[token->data.name_tab_index]);
            add_son_node(active_node, rand_node);

            assert_token(add_son_node(rand_node, expr()), "Missing expression");
            assert_token(get_token()->type == oper_sep, "Missing comma");
            token_index++;
            assert_token(add_son_node(rand_node, expr()), "Missing expression after comma");
            break;
        }
        // "(" EXPR ")" | "|" EXPR "|"
        case oper_abs:
        case oper_lpar:
            assert_token(add_son_node(active_node, expr()), "Missing expression");
            if (token->type == oper_abs) {
                assert_token(get_token()->type == oper_abs, "missing right absolut");
            } else {
                assert_token(get_token()->type == oper_rpar, "missing right bracket");
            }
            token_index++;
            break;
        // ZIFFER {ZIFFER} ["." {ZIFFER}]
        // done by lexer => copy value of token
        case oper_const: {
            treenode_t *const_node = new_tree_node();
            const_node->type = oper_const;
            const_node->d.val = get_token()->data.val;
            token_index++;
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
    int last_token_index = token_index;
    type_t type = get_token()->type;
    switch (type) {
        case keyw_walk:
        case keyw_jump:
            token_index++;
            switch (get_token()->type) {
                case keyw_back:
                case keyw_home:
                    break;
                default:
                    token_index--;
                    break;
            }
            break;
        case keyw_turn:
            switch (get_token()->type) {
                case keyw_left:
                case keyw_right:
                    token_index++;
                    break;
                default:
                    token_index--;
                    break;
            }
            break;
        case keyw_direction:
            token_index++;
            break;
        case keyw_color:
            token_index++;
            assert_token(color(), "missing value for color");
            break;
        case keyw_clear:
        case keyw_stop:
        case keyw_finish:
            token_index++;
            break;
        case keyw_path:
            token_index++;
            assert_token(name(false), "missing name for path");
            if (get_token()->type == oper_lpar) {
                token_index++;
                fill_args(NULL); // todo: insert parent_node instead of NULL >_<
                assert_token(get_token()->type == oper_rpar, "missing closing parenthesis");
                token_index++;
            }
            break;
        default:
            return NULL;
    }

    assert_token(expr(), "missing expression");


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
    token_index++;

    treenode_t *node = new_tree_node();
    node->type = type;
    if (type != keyw_mark) {
        if (get_token()->type == keyw_mark) {
            token_index++;
            node->d.walk = keyw_mark;
        } else {
            token_index = last_token_index;
            return NULL;
        }
    }

    return node;
}

treenode_t *cmd_calc() {

}


treenode_t *cmd_if() {
    if (get_token()->type != keyw_if) {
        return NULL;
    }
    token_index++;

    treenode_t *node = new_tree_node();
    node->type = keyw_if;

    assert_token(add_son_node(node, cond()), "missing condition");
    assert_token(get_token()->type == keyw_then, "missing then keyword");
    token_index++;

    fill_statements(node);

    if (get_token()->type == keyw_else) {
        token_index++;
        fill_statements(node);
    }

    assert_token(get_token()->type == keyw_endif, "missing endif keyword");
    token_index++;

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
    assert_token(get_token()->type == keyw_times, "missing times keyword");
    token_index++;

    fill_statements(node);
    assert_token(get_token()->type == keyw_done, "missing done keyword");
    token_index++;

    return node;
}

treenode_t *cmd_counter() {

}

treenode_t *cmd_while() {
    if (get_token()->type != keyw_while) {
        return NULL;
    }
    token_index++;

    treenode_t *node = new_tree_node();
    node->type = keyw_while;

    assert_token(add_son_node(node, cond()), "missing condition for while cmd");
    assert_token(get_token()->type == keyw_do, "missing do keyword");
    token_index++;

    fill_statements(node);
    assert_token(get_token()->type == keyw_done, "missing done keyword");
    token_index++;

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
    assert_token(get_token()->type == keyw_until, "missing until keyword");
    token_index++;

    assert_token(add_son_node(node, cond()), "missing condition for repeat cmd");

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
