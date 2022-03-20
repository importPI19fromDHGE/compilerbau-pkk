//
// Created by max on 14.03.22.
//

#include <ctype.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>

#include "lexer.h"

// global `char *input_buf`

void lex(void) {
    char *token;
    int pos_word_start = 0;
    int token_length = 0; // für malloc und String-OPs
    // debugging
    int line_no = 1; // Zeilennummer für Fehlerbehandlung
    int col_no = 0;  // Spaltennummer für Fehlerbehandlung
    bool now_reading_word = false; // benötigt, um zu erkennen, ob wir ein Wort lesen, weil mehrere Whitespaces erlaubt sind

    for (int pos = 0; pos < input_buf_length; pos++) {
        // todo: comments
        col_no++;

        if (isspace(input_buf[pos])) { // current char is whitespace
            if (now_reading_word) { // previous char was not whitespace aka a word just ended
                now_reading_word = false;
                token_length = pos - pos_word_start;
                token = malloc(sizeof(char) * token_length); //TODO Fehlerbehandlung
                for (int i = 0; i < token_length; i++) {
                    token[i] = input_buf[pos_word_start + i];
                }
                // current token string done, print it
                printf("%s\n", token);
                // todo: handle finished token
            }

            if (input_buf[pos] == '\n') { // Sonderfall Newline
                col_no = 1;
                line_no++;
            }
            continue;
        } else { // current char is not whitespace
            if (!now_reading_word) { // a word just started
                pos_word_start = pos;
            }
            now_reading_word = true;
        }
    }
}