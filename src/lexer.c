//
// Created by max on 14.03.22.
//

#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "lexer.h"

// global `char *input_buf` is the input source code

/// Array of delimiting characters
delimiter_t delimiters[MAX_DELIMITER] = {
        {'>',  true},
        {'<',  true},
        {' ',  false},
        {'\n', false},
        {'\t', false},
        {'"',  false},
        {'=',  false},
        {'+',  false},
        {'-',  false},
        {'*',  false},
        {'/', false},
        {'^', false},
        {'(', false},
        {')', false},
        {'|', false}
};

/// todo: documentation
word_t *words;
int word_count = 0;

int line_nr = 1, col_nr = 1;

bool is_delimiter(char c) {
    bool is_delimiter = false;
    for (int i = 0; i < MAX_DELIMITER; ++i) {
        if (delimiters[i].delimiting_char == c) {
            is_delimiter = true;
            break;
        }
    }
    return is_delimiter;
}

int read_word(const int start_pos) { // start_pos ist globales Offset zu input_buf
    int word_len = 0;

    // todo: document buffer overflow check
    while ((start_pos + word_len) <= input_buf_length && !is_delimiter(input_buf[start_pos + word_len])) {
        word_len++;
    }

    if (word_len == 0) {
        word_len = 1;
    }

    // handle comments
    if (input_buf[start_pos] == '"') {
        while ((start_pos + word_len) <= input_buf_length && input_buf[start_pos + word_len] != '\n') {
            word_len++;
        }
        return start_pos + word_len;
    }
    // ignore spaces
    if (isspace(input_buf[start_pos])) {
        if (input_buf[start_pos] == '\n') {
            line_nr++;
            col_nr = 1;
        } else {
            col_nr++;
        }
        return start_pos + 1;
    }

    // fill struct
    word_t word = {
        input_buf + start_pos,
        word_len,
        line_nr,
        col_nr
    };

    col_nr += word_len;

    // write to words array and increment word_count for next iteration
    words[word_count++] = word;
    return start_pos + word_len;
}

type_t recognise_token_type(const char *word) {
    type_t type;

    // set matching type if the first char is a delimiter
    switch (word[0]) {
        case '(': type = oper_lpar;
            break;
        case ')': type = oper_rpar;
            break;
        case ',': type = oper_sep;
            break;
        case '|': type = oper_abs;
            break;
        case '^': type = oper_pow;
            break;
        case '*': type = oper_mul;
            break;
        case '/': type = oper_div;
            break;
        case '+': type = oper_add;
            break;
        case '-': type = oper_sub; // als Token sind unäre (-) auch oper_sub, siehe turtle-types.h
            break;
        case '=': type = oper_equ;
            break;
        case '<': //type = oper_less; // kann oper_less oder oper_nequ oder oper_lequ sein
            switch (word[1]) {
                case '=': type = oper_lequ;
                    break;
                case '>': type = oper_nequ;
                    break;
                default: type = oper_less;
                    break;
            }
            break;
        case '>': // kann oper_grtr oder oper_gequ sein
            type = (input_buf[1] == '=') ? oper_gequ : oper_grtr;
            break;
        default: // word is from type name_any or oper_const
            type = oper_const;
            // check if oper_const contains any alphabetic char
            //  E/e is ignored due to task specifications
            for (int i = 0; word[i] != '\0'; i++) {
                if ((word[i] >= 'a' && word[i] <= 'z')
                    || (word[i] >= 'A' && word[i] <= 'Z')) {
                    // drop out if an alphabetic char was found
                    type = name_any;
                    break;
                }
            }
            break;
    }

    // type already found
    if (type != name_any) {
        return type;
    }

    // search a matching name in name_tab
    for (int i = 0; i < name_tab_length; i++) { // lineare search through name_tab
        if (strcmp(word, name_tab[i].name) == 0) { // word matches
            type = name_tab[i].type;
            break;
        }
    }

    // type in name_tab found
    if (type != name_any) {
        return type;
    }

    name_tab[name_tab_length++] = (nameentry_t) {type, word, {} };
    return type;
}


void lex(void) {
    insertArray(&token_stream, (token_t) {tok_bofeof});
    words = malloc(20000 * sizeof(word_t));

    int current_pos = 0;

    while (current_pos < input_buf_length) {
        current_pos = read_word(current_pos);
    }
    // ab hier gesamter Input getrennt

    for (int i = 0; i < word_count; i++) {
        char *word = malloc((words[i].length + 1) * sizeof(char));
        word[words[i].length] = '\0';
        strncpy(word, words[i].word_ptr, words[i].length);
        type_t token_type =  recognise_token_type(word);


////////// rly unsure if this is useful to the whole situation but here are my thoughts
        // todo: open problem to solve
        //  - find solution to save numbers
        //  - some const need a ref to name_tab but recognise_token_type only returns type
        //      - for name_any we can just lookup the last name_tab entry cause we have to add every name_any
        // fixme: current solution => added nodedata_t to token instead of name_tab_index so doubles can be saved

        switch (token_type) {
            case keyw_walk: case keyw_back: case keyw_home: case keyw_mark:
                insertArray(&token_stream, (token_t) {
                    token_type,
                    words[i].pos,
                    (nodedata_t) token_type
                });
                break;
            // todo: constants which need ref to name_tab
            // case name_pvar_ro: case name_pvar_rw:
            // case name_math_sin: case name_math_cos: case name_math_tan: case name_math_sqrt: case name_math_rand:
            //    insertArray(&token_stream,(token_t) {
            //            token_type,
            //            words[i].pos,
            //            (nodedata_t) &name_tab[???]
            //    });
            //    break;
            case name_any:
                // if the type is name_any the word was added to the name_tab
                // so the index is the len of name_tab minus 1
                insertArray(&token_stream, (token_t) {
                    token_type,
                    words[i].pos,
                    (nodedata_t) &name_tab[name_tab_length - 1]
                });
                break;
            case oper_const: {
                // conversion of word to double
                char *_ignore;
                insertArray(&token_stream, (token_t) {
                    token_type,
                    words[i].pos,
                    (nodedata_t) strtod(words[i].word_ptr, &_ignore)
                });
                break;
            }
            default:
                insertArray(&token_stream, (token_t) {
                    token_type,
                    words[i].pos
                });
                break;
        }
    }
////////// end
    insertArray(&token_stream, (token_t) {tok_bofeof});
}
