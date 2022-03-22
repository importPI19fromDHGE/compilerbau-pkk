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
    bool now_reading_comment = false; // wird gerade ein Kommentar verarbeitet?

    int being_end_token_index = ++name_tab_length;
    name_tab[being_end_token_index] = (nameentry_t) {tok_bofeof, "", {}};

    insertArray(&token_stream, &name_tab[name_tab_length]);

    for (int pos = 0; pos < input_buf_length; pos++) {  // handle comments
        if (input_buf[pos] == '\"') {
            now_reading_comment = true;
        }
        if (now_reading_comment) {
            if (input_buf[pos] == '\n') {
                col_no = 0;
                line_no++;
                now_reading_comment = false;
            }
            continue;
        }

        col_no++;

        type_t type = name_any;

        switch (input_buf[pos]) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                break;
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
                type = oper_sub; // als Token sind unäre - auch oper_sub, siehe turtle-types.h
                break;
            case '=':
                type = oper_equ;
                break;
            case '<':
                //type = oper_less; // kann oper_less oder oper_nequ oder oper_lequ sein
                switch(input_buf[pos+1]) {
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
            case '>':
                // type = oper_grtr; // kann oper_grtr oder oper_gequ sein
                if (input_buf[pos+1] == '=') {
                    type = oper_gequ;
                } else {
                    type = oper_grtr;
                }
                break;
        }

        if (isspace(input_buf[pos])) { // current char is whitespace
            if (now_reading_word) { // previous char was not whitespace aka a word just ended
                now_reading_word = false;
                token_length = pos - pos_word_start;
                token = malloc(sizeof(char) * (token_length + 1)); // TODO: Fehlerbehandlung
                strncpy(token, input_buf + pos_word_start, token_length);
                token[token_length] = '\0';

                // current token string done
                // todo: handle finished token
                int name_index = 0; // this stores the name_tab array index
                bool token_found = false;

                // name_index ist bei nicht gefundenen Tokens am Ende des Arrays
                for(name_index = 0; name_index < name_tab_length; name_index++) { // lineare Suche durch die Nametab
                    if (strcmp(token, name_tab[name_index].name) == 0) { // token matches
                        token_found = true;
                        break;
                    }
                }

                if (!token_found) { // name is not in name_tab, insert it
                    name_index++;
                    nameentry_t new_token_entry = {
                            type,
                            token,
                            {}
                    };
                    name_tab[name_index] = new_token_entry;
                }

                // token_stream mit name_tab index befüllen;
                insertArray(&token_stream, &(name_tab[name_index]));
                // current token string done, print it
                printf("(Ln: %d Col: %d) Read: %s Save: %s\n", line_no, col_no, name_tab[name_index].name, token_stream.array[token_stream.used-1].name);
                free(token);
            }

            if (input_buf[pos] == '\n') { // Sonderfall Newline
                col_no = 0;
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