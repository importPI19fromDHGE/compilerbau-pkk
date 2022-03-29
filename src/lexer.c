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
        {'/',  false},
        {'^',  false},
        {'(',  false},
        {')',  false},
        {'|',  false}
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

int find_in_name_tab(const char *word) {
    for (int index = 0; index < name_tab_length; index++) { // lineare search through name_tab
        if (strcmp(word, name_tab[index].name) == 0) { // word matches
            return index;
        }
    }
    return -1;
}

int insert_in_name_tab(nameentry_t name_entry) {
    name_tab[name_tab_length] = name_entry;
    name_tab_length++;
    return name_tab_length - 1;
}

type_t recognise_token_type(const char *word) {
    type_t type;

    // set matching type if the first char is a delimiter
    switch (word[0]) {
        case '(':
            type = oper_lpar;
            break;
        case ')':
            type = oper_rpar;
            break;
        case ',':
            type = oper_sep;
            break;
        case '|':
            type = oper_abs;
            break;
        case '^':
            type = oper_pow;
            break;
        case '*':
            type = oper_mul;
            break;
        case '/':
            type = oper_div;
            break;
        case '+':
            type = oper_add;
            break;
        case '-':
            type = oper_sub; // als Token sind unäre (-) auch oper_sub, siehe turtle-types.h
            break;
        case '=':
            type = oper_equ;
            break;
        case '<': //type = oper_less; // kann oper_less oder oper_nequ oder oper_lequ sein
            switch (word[1]) {
                case '=':
                    type = oper_lequ;
                    break;
                case '>':
                    type = oper_nequ;
                    break;
                default:
                    type = oper_less;
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

    int name_entry_index = find_in_name_tab(word);

    if (name_entry_index < 0) {
        name_entry_index = insert_in_name_tab((nameentry_t) {type, word, {}});
    }

    return name_tab[name_entry_index].type;
}

void insert_in_token_stream(const word_t *word_entry) {
    // slice word of length word_length of the given word_ptr
    char *word = malloc((word_entry->length + 1) * sizeof(char));
    word[word_entry->length] = '\0';
    strncpy(word, word_entry->word_ptr, word_entry->length);

    type_t type = recognise_token_type(word);

    switch (type) {
        case name_any:
        case name_pvar_ro:
        case name_pvar_rw:
        case name_math_sin:
        case name_math_cos:
        case name_math_tan:
        case name_math_sqrt:
        case name_math_rand:
            insertArray(&token_stream, (token_t) {
                    type,
                    word_entry->pos,
                    {.name_tab_index = find_in_name_tab(word)}
            });
            break;
        case oper_const:
            // conversion of word to double
            insertArray(&token_stream, (token_t) {
                    type,
                    word_entry->pos,
                    {.val = strtod(word_entry->word_ptr, NULL)}
            });
            break;
        default:
            insertArray(&token_stream, (token_t) {type, word_entry->pos});
            break;
    }

    // free word if it isn't saved as name_tab entry
    if (type != name_any) {
        free(word);
    }
}

void lex(void) {
    insertArray(&token_stream, (token_t) {tok_bofeof, 0, 0});
    words = malloc(20000 * sizeof(word_t));

    int current_pos = 0;
    while (current_pos < input_buf_length) {
        current_pos = read_word(current_pos);
    }

    for (int i = 0; i < word_count; i++) {
        insert_in_token_stream(&(words[i]));
    }

    insertArray(&token_stream, (token_t) {tok_bofeof, 0, 0});
}
