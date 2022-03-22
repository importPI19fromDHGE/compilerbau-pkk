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
    char *token_str;
    int pos_word_start = 0;
    int token_length = 0; // für malloc und String-OPs
    // debugging
    int line_no = 1; // Zeilennummer für Fehlerbehandlung
    int col_no = 0;  // Spaltennummer für Fehlerbehandlung
    bool now_reading_word = false; // benötigt, um zu erkennen, ob wir ein Wort lesen, weil mehrere Whitespaces erlaubt sind
    bool word_has_ended = true; // auf Trennzeichen gestoßen, nicht zwingend Whitespace
    bool now_reading_comment = false; // wird gerade ein Kommentar verarbeitet?
    bool word_is_number = true;

    insertArray(&token_stream, (token_t) {tok_bofeof, -1});

    for (int pos = 0; pos < input_buf_length; pos++) {  // handle comments
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
        word_has_ended = true;
        word_is_number = true;

        // alles in diesem Switch ist ein potenzielles Trennzeichen
        switch (input_buf[pos]) { // Typerkennung
            case ' ':
            case '\t':
                break;
            case '\n':
                col_no = 0;
                line_no++;
                break;
            case '"':
                now_reading_comment = true;
                continue; // comment handling is above
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
            case '<':
                //type = oper_less; // kann oper_less oder oper_nequ oder oper_lequ sein
                switch (input_buf[pos + 1]) {
                    case '=':
                        type = oper_lequ;
                        pos++;
                        break;
                    case '>':
                        type = oper_nequ;
                        pos++;
                        break;
                    default:
                        type = oper_less;
                        break;
                }
                break;
            case '>':
                // kann oper_grtr oder oper_gequ sein
                if (input_buf[pos + 1] == '=') {
                    type = oper_gequ;
                    pos++;
                } else {
                    type = oper_grtr;
                }
                break;
            default: // kein Trennzeichen gefunden
                word_has_ended = false;
                break;
        }

        int name_index = -1; // this stores the name_tab array index
        if (word_has_ended) {
            token_str = malloc(sizeof(char) * (token_length + 1)); // TODO: Fehlerbehandlung
            strncpy(token_str, input_buf + pos_word_start, token_length);
            token_str[token_length] = '\0';
        }

        if (type == name_any) {
            if (!isdigit(input_buf[pos])) {
                word_is_number = false;
            }
            if (now_reading_word && word_has_ended) {
                // handle word token_str
                now_reading_word = false;
                token_length = pos - pos_word_start;

                // todo: number?

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // this: insert to name_table?
                // current token_str string done, handle it
                bool token_found = false;

                // name_index ist bei nicht gefundenen Tokens am Ende des Arrays
                for (name_index = 0; name_index < name_tab_length; name_index++) { // lineare Suche durch die Nametab
                    if (strcmp(token_str, name_tab[name_index].name) == 0) { // token_str matches
                        token_found = true;
                        break;
                    }
                }

                if (!token_found) { // name is not in name_tab, insert it
                    name_index++;
                    nameentry_t new_token_entry = {
                            type,
                            token_str,
                            {}
                    };
                    name_tab[name_index] = new_token_entry;
                }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            } else if (!now_reading_word) { // a word just started
                pos_word_start = pos;
                now_reading_word = true;
            }
        }

        // save token_str with type
        if (word_has_ended) {
            // token_t befüllen
            token_t token = {type, name_index };
            // token_t in token_stream array speichern
            insertArray(&token_stream, token);
            // current token_str string done, print it
            printf("(Ln: %d Col: %d) Saved: [%s] with possible nameentry index: %d\n",
                   line_no,
                   col_no,
                   token_str,
                   token.nameentry_index
            );
            free(token_str);
        }
    }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// OBSOLETE
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//        if (isspace(input_buf[pos])) { // current char is whitespace
//            if (now_reading_word) { // previous char was not whitespace aka a word just ended
//                now_reading_word = false;
//                token_length = pos - pos_word_start;
//                token_str = malloc(sizeof(char) * (token_length + 1)); // TODO: Fehlerbehandlung
//                strncpy(token_str, input_buf + pos_word_start, token_length);
//                token_str[token_length] = '\0';
//
//                // current token_str string done, handle it
//                int name_index = 0; // this stores the name_tab array index
//                bool token_found = false;
//
//                // name_index ist bei nicht gefundenen Tokens am Ende des Arrays
//                for(name_index = 0; name_index < name_tab_length; name_index++) { // lineare Suche durch die Nametab
//                    if (strcmp(token_str, name_tab[name_index].name) == 0) { // token_str matches
//                        token_found = true;
//                        break;
//                    }
//                }
//
//                if (!token_found) { // name is not in name_tab, insert it
//                    name_index++;
//                    nameentry_t new_token_entry = {
//                            type,
//                            token_str,
//                            {}
//                    };
//                    name_tab[name_index] = new_token_entry;
//                }

                // token_stream mit name_tab index befüllen;
//                insertArray(&token_stream, &(name_tab[name_index]));
//                // current token_str string done, print it
//                printf("(Ln: %d Col: %d) Read: %s Save: %s\n",
//                    line_no,
//                    col_no,
//                    name_tab[name_index].name,
//                    token_stream.array[token_stream.used-1].name
//                );
//                free(token_str);
//            }

//            if (input_buf[pos] == '\n') { // Sonderfall Newline
//                col_no = 0;
//                line_no++;
//            }
//            continue;
//        } else { // current char is not whitespace
//            if (!now_reading_word) { // a word just started
//                pos_word_start = pos;
//            }
//            now_reading_word = true;
//        }
//    }
/////////////////////////////////////////////////////////////////////////////////////////////////////////// NOT OBSOLETE
    insertArray(&token_stream, (token_t) {tok_bofeof, -1});
}