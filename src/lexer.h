//
// Created by max on 14.03.22.
//

#ifndef COMPILERBAU_PKK_LEXER_H
#define COMPILERBAU_PKK_LEXER_H
#define MAX_DELIMITER 15

#include "turtle.h"

void lex(void);

/// contains a word with metadata
typedef struct {
    char *word_ptr;
    int length;

    int col_nr; // counts from 1
    int line_nr; // counts from 1
} word_t;

/// Represents a delimiting character and whether or not lookahead is required for proper type identification
typedef struct {
    char delimiting_char;
    bool needs_lookahead;
} delimiter_t;

word_t *words;

/// Reads a word and writes information to `words`
/// \param current_pos start of the next word
/// \return position for the next `read_word` call
int read_word(int current_pos);

/// Determines the type of a token.
/// \param token
void recognise_token_type(const char *token);

/// Performs a linear search over the nameentry_t `name_tab`, looking for a given token
/// \param token a \0-terminated String, the token to search for
/// \return The index of the token in the `nametab`; -1 if not found
int find_in_nametab(const char *token);

/// Insert a new name into the nametab
/// \param nameentry
/// \return the nametab-index of the new entry
int insert_in_nametab(nameentry_t nameentry);

/// Performs a linear search over the delimiters-Array
/// \param c The character to check
/// \return True if c is a delimiter, False otherwise
bool is_delimiter(char c);

#endif //COMPILERBAU_PKK_LEXER_H
