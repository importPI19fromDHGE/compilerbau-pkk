//
// Created by pkk on 29.03.22.
//

#ifndef COMPILERBAU_PKK_PARSER_H
#define COMPILERBAU_PKK_PARSER_H

#ifndef COMPILERBAU_PKK_DYNAMIC_ARRAY_H
#include "dynamic_array.h"
#endif

extern treenode_t *syntax_tree;
extern Array *token_stream;

// sollte nur hochgezählt werden, wenn es auch wirklich verarbeitet wurde
int token_index;

void parse() {

}

void program();
void pathdef();
void calcdef();
void params();


void name(); // == var or name idk
void var();
void name_var_any();
void determine_name_var();

void color();
void cond();
void cond_s();
void expr();
void term();
void faktor();
void operand();
void args();
void statements();
void statement();
void draw_cmd();
void calc_cmd();
void if_cmd();
void times_cmd();
void cnt_cmd();
void while_cmd();
void repeat();

#endif