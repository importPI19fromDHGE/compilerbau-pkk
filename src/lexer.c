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
                token = malloc(sizeof(char) * (token_length + 1)); //TODO Fehlerbehandlung
                /*for (int i = 0; i < token_length; i++) {
                    token[i] = input_buf[pos_word_start + i];
                }*/
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
//                    name_tab[name_index];
                }

                // token_stream mit name_tab index befüllen;
                insertArray(&token_stream, &(name_tab[name_index]));
                // current token string done, print it
                printf("Read: %s Save: %s\n", name_tab[name_index].name, token_stream.array[token_stream.used-1].name);
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