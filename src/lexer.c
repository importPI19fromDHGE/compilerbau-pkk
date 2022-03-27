//
// Created by max on 14.03.22.
//

#include <ctype.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>

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

void recognise_token_type(const char *token) {
    // todo: is this is a number? --> oper_const
    // todo: is this in the nametab? --> add if it isn't
    //
}

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

int read_word(const int start_pos) {// start_pos ist globales Offset zu input_buf
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
        (word_len > 0) ? word_len + 1 : word_len,
        col_nr,
        line_nr
    };

    col_nr += word_len;

    // write to words array
    words[word_count] = word;
    printf("[ln: %d, col: %d] %.*s\n", word.line_nr, word.col_nr, word_len, word.word_ptr);

    word_count++;

    return start_pos + word_len;
}

void lex(void) {
    insertArray(&token_stream, (token_t) {tok_bofeof, -1});
    words = malloc(20000 * sizeof(word_t));

    int current_pos = 0;

    while (current_pos < input_buf_length) {
        current_pos = read_word(current_pos);
    }
    // ab hier gesamter Input getrennt



    insertArray(&token_stream, (token_t) {tok_bofeof, -1});
}
