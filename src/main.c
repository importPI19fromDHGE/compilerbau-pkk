
// Turte-Graphics-Compiler:
// Hauptprogramm und programmweite Hilfsfunktionen
//
// Klaus Kusche 2021, 2022

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include "turtle.h"
#include "lexer.h"
#include "debug.h"

// Global: Der Sourcefile & der Programmname
char *input_buf;
long input_buf_length;
const char *prog_name;
Array token_stream;

// Array-Index des Beginns der aktuellen Zeile

// Prüfe ob malloc/calloc/realloc erfolgreich war:
// Fehlermeldung und Programmende, wenn p gleich NULL ist
// what ... was wurde gerade angelegt?
// pos ... für welche Stelle im Source?
void mem_check(const void *p, const char *what, const srcpos_t *pos) {
    if (p == NULL) {
        fprintf(stderr, "%s: Fehler beim Anlegen von Speicher für %s "
                        "(Zeile %d, Spalte %d): %s\n",
                prog_name, what, pos->line, pos->col, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

// Ausgabe eines Fehlers im Turtle-Programm an Stelle pos & Programmende
// (mit variabel vielen Parametern wie printf)
void code_error(const srcpos_t *pos, const char *format, ...) {
    va_list arg_p;

    printf("Fehler in Zeile %d, Spalte %d: ", pos->line, pos->col);
    va_start(arg_p, format);
    vprintf(format, arg_p);
    va_end(arg_p);
    putchar('\n');
    exit(EXIT_FAILURE);
}

treenode_t *lex_parse(void){
    initArray(&token_stream, 1000);
    lex();

    // debug: output to review the token_stream
    //  values surrounded by <> are variables and should have a link to name_tab => bug:
    for (int i = 0; i < token_stream.used; i++ ) {
        token_t token = token_stream.array[i];
        printf("[ln: %2d, col: %2d, type: %16s] ",
               token.pos.line, token.pos.col, DEBUG_TURTLE_TYPE_NAMES[token.type].name);
        switch (token.type) {
            // used in combination with walk and jump => cause only token we don't care yet
            case keyw_walk: case keyw_back: case keyw_home: case keyw_mark:
                printf("%s\n", DEBUG_TURTLE_TYPE_NAMES[token.value.walk].name);
                break;
            case name_any:
            // case name_pvar_ro: case name_pvar_rw: case name_math_sin: case name_math_cos:
            // case name_math_tan: case name_math_sqrt: case name_math_rand:
                printf("%s\n", token.value.p_name->name);
                break;
            case oper_const:
                printf("%f\n", token.value.val);
                break;
            default:
                printf("%s\n", DEBUG_TURTLE_TYPE_NAMES[token.type].value);
        }
    }

    //todo: parse
}

void read_from_file(FILE *src_file) {
    // read src_file
    // https://stackoverflow.com/questions/174531/how-to-read-the-content-of-a-file-to-a-string-in-c
    if (src_file) {
        fseek(src_file, 0, SEEK_END);
        input_buf_length = ftell(src_file);
        fseek(src_file, 0, SEEK_SET);
        input_buf = malloc(input_buf_length);
        if (input_buf) {
            fread(input_buf, 1, input_buf_length, src_file);
        }
    }
}

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Aufruf: %s Programm-Datei [Zahlen ...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (argc > 11) {
        fprintf(stderr, "Aufruf: %s Programm-Datei [Zahlen ...]\n"
                        "Höchstens 9 Zahlen!\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        fprintf(stderr, "%s: Fehler beim Öffnen von %s zum Lesen: %s\n",
                argv[0], argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }

    read_from_file(file);
    fclose(file);

    prog_name = argv[0];

    lex_parse();
    // evaluate(lex_parse(), argc - 2, &(argv[2]));

    exit(EXIT_SUCCESS);
}
