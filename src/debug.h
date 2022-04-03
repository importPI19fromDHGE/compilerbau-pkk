//
// Created by photovoltex on 28.03.22.
//

#ifndef COMPILERBAU_PKK_DEBUG_H
#define COMPILERBAU_PKK_DEBUG_H
#define DEBUG

typedef struct {
    char *name;
    char *value;
} DEBUG_TURTLE_TYPE_NAMES_T;

// 1:1 match for enum type_t
// DEBUG_TURTLE_TYPE_NAMES[<type_t>] will return a struct with the type name saved as string (name) and a debug value
DEBUG_TURTLE_TYPE_NAMES_T DEBUG_TURTLE_TYPE_NAMES[76] = {
        {"name_any","<any>"},
        {"name_var","<var>"},
        {"name_path","<name of path-fct>"},
        {"name_calc","<name of cal-fct>"},
        {"name_glob","<user defined global var>"},
        {"name_pvar_ro","<read only var>"},
        {"name_pvar_rw","<read-write var>"},
        {"name_math_sin","<math sin>"},
        {"name_math_cos","<math cos>"},
        {"name_math_tan","<math tan>"},
        {"name_math_sqrt","<math sqrt>"},
        {"name_math_rand","<math rand>"},
        {"keyw_dummy","'dummy'"},
        {"keyw_walk","'walk'"},
        {"keyw_jump","'jump'"},
        {"keyw_back","'back'"},
        {"keyw_home","'home'"},
        {"keyw_turn","'turn'"},
        {"keyw_left","'left'"},
        {"keyw_right","'right'"},
        {"keyw_direction","'direction'"},
        {"keyw_clear","'clear'"},
        {"keyw_stop","'stop'"},
        {"keyw_finish","'finish'"},
        {"keyw_path","'path'"},
        {"keyw_color","'color'"},
        {"keyw_store","'store'"},
        {"keyw_in","'in'"},
        {"keyw_add","'add'"},
        {"keyw_to","'to'"},
        {"keyw_sub","'sub'"},
        {"keyw_from","'from'"},
        {"keyw_mul","'mul'"},
        {"keyw_by","'by'"},
        {"keyw_div","'div'"},
        {"keyw_mark","'mark'"},
        {"keyw_if","'if'"},
        {"keyw_then","'then'"},
        {"keyw_else","'else'"},
        {"keyw_endif","'endif'"},
        {"keyw_do","'do'"},
        {"keyw_times","'times'"},
        {"keyw_done","'done'"},
        {"keyw_counter","'counter'"},
        {"keyw_downto","'downto'"},
        {"keyw_step","'step'"},
        {"keyw_while","'while'"},
        {"keyw_repeat","'repeat'"},
        {"keyw_until","'until'"},
        {"keyw_endpath","'endpath'"},
        {"keyw_calculation","'calculation'"},
        {"keyw_returns","'returns'"},
        {"keyw_endcalc","'endcalc'"},
        {"keyw_begin","'begin'"},
        {"keyw_end","'end'"},
        {"keyw_and","'and'"},
        {"keyw_or","'or'"},
        {"keyw_not","'not'"},
        {"tok_bofeof","{BEGIN_OR_END}"},
        {"oper_const","<const number>"},
        {"oper_lpar","'('"},
        {"oper_rpar","')'"},
        {"oper_sep","','"},
        {"oper_abs","'|'"},
        {"oper_pow","'^'"},
        {"oper_mul","'*'"},
        {"oper_div","'/'"},
        {"oper_add","'+'"},
        {"oper_sub","'-'"},
        {"oper_neg","'-'"},
        {"oper_equ","'='"},
        {"oper_nequ","'<>'"},
        {"oper_less","'<'"},
        {"oper_lequ","'<='"},
        {"oper_grtr","'>'"},
        {"oper_gequ","'>='"}
};

#endif //COMPILERBAU_PKK_DEBUG_H
