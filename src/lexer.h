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

    srcpos_t pos;
} word_t;

/// Represents a delimiting character and whether or not lookahead is required for proper type identification
typedef struct {
    char delimiting_char;
    bool needs_lookahead;
} delimiter_t;

extern int word_count;
extern word_t *words;


/// Performs a linear search over the delimiters-Array
/// \param c The character to check
/// \return True if c is a delimiter, False otherwise
bool is_delimiter(char c);

/// Reads a word and writes information to `words`
/// \param start_pos globales Offset zu input_buf
/// \returns position for the next `read_word` call
int read_word(int start_pos);


/// Performs a linear search over the nameentry_t `name_tab`, looking for a given token
/// \param word a \0-terminated String, the token to search for
/// \return The index of the token in the `nametab`; -1 if not found
int find_in_name_tab(const char *word);

/// Insert a new name into the nametab
/// \param nameentry
/// \return the nametab-index of the new entry
int insert_in_name_tab(nameentry_t nameentry);

/// Determines the type of a given word.
/// \param word for which the token type should be determined.
///             Has to be a valid c string.
/// \returns token type of given word.
///           if name_any is returned the index of name_tab is the last name_tab_length - 1.
type_t recognise_token_type(const char *word);

#endif //COMPILERBAU_PKK_LEXER_H
