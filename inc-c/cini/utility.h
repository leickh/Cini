
#ifndef CINI_UTILITY_H
#define CINI_UTILITY_H

#include <stdbool.h>
#include <stdint.h>

// ==> Allocators

typedef struct CiniArena CiniArena;

struct CiniArena
{
    uint32_t capacity;
    uint32_t usage;
    void *allocation;

    CiniArena *continuation;
};

CiniArena * cini_new_arena(
    uint32_t capacity
);

void cini_free_arena(
    CiniArena *arena
);

void * cini_arena_alloc(
    CiniArena *arena,
    uint32_t length
);

char * cini_arena_copy_string(
    CiniArena *arena,
    const char *string
);



// ==> String/Character Utilities

typedef enum
{
    CINI_ASCII_EXCLAMATION_MARK,
    CINI_ASCII_DOUBLE_QUOTATION_MARK,
    CINI_ASCII_HASH_SIGN,
    CINI_ASCII_DOLLAR_SIGN,
    CINI_ASCII_PERCENT_SIGN,
    CINI_ASCII_AMPERSAND,
    CINI_ASCII_SINGLE_QUOTATION_MARK,
    CINI_ASCII_OPENING_PARENTHESIS,
    CINI_ASCII_CLOSING_PARENTHESIS,
    CINI_ASCII_ASTERISK,
    CINI_ASCII_PLUS,
    CINI_ASCII_COMMA,
    CINI_ASCII_MINUS,
    CINI_ASCII_POINT,
    CINI_ASCII_SLASH,
    CINI_ASCII_COLON,
    CINI_ASCII_SEMICOLON,
    CINI_ASCII_SMALLER_THAN,
    CINI_ASCII_EQUALS_SIGN,
    CINI_ASCII_BIGGER_THAN,
    CINI_ASCII_QUESTION_MARK,
    CINI_ASCII_AT_SIGN,
    CINI_ASCII_OPENING_SQUARE_BRACKET,
    CINI_ASCII_BACKSLASH,
    CINI_ASCII_CLOSING_SQUARE_BRACKET,
    CINI_ASCII_CIRCUMFLEX,
    CINI_ASCII_UNDERSCORE,
    CINI_ASCII_TICK,
    CINI_ASCII_OPENING_CURLY_BRACE,
    CINI_ASCII_VERTICAL_BAR,
    CINI_ASCII_CLOSING_CURLY_BRACE,
    CINI_ASCII_TILDE,

    CINI_ASCII_NOT_A_SIGN

} CiniAsciiSign;

uint32_t cini_extract_utf8(
    const char *string,
    uint32_t offset,
    uint32_t *remaining
);

bool cini_check_newline(const char *string, uint32_t offset, uint32_t *next);
bool cini_is_lowercase(uint32_t rune);
bool cini_is_uppercase(uint32_t rune);
bool cini_is_letter(uint32_t rune);
bool cini_is_digit(uint32_t rune);
bool cini_is_sign(uint32_t rune);

bool cini_rune_is_ascii_special(uint32_t rune);
CiniAsciiSign cini_rune_to_sign_enum(uint32_t rune);


#endif // CINI_UTILITY_H

