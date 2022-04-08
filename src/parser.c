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
    // todo: oh boi
    treenode_t *statement_to_add = NULL;

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