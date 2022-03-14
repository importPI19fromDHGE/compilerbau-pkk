//
// Created by max on 14.03.22.
//

#include <ctype.h>
#include <malloc.h>
#include "lexer.h"

// global `char *input_buf`

void lex(void) {
    char *token;
    int pos = 0;
    int line_no = 1; // Zeilennummer für Fehlerbehandlung
    int col_no = 1;  // Spaltennummer für Fehlerbehandlung

    for (int i = 0; i < input_buf_length; i++) {
        // todo: comments
        if (isspace(input_buf[i])) {
            col_no++;
            if (input_buf[i] == '\n') { // Sonderfall Newline
                col_no = 1;
                line_no++;
            }
            // token = malloc(col_no);
            // token[col_no - 1] = input_buf[i];
            continue;
        }
    }

}