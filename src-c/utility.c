#include <cini/utility.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// ==> Mathematics

int64_t cini_max_i64(
    int64_t first,
    int64_t second
) {
    if (first > second)
    {
        return first;
    }
    return second;
}



// ==> Allocators

CiniArena * cini_new_arena(
    uint32_t capacity,
    CiniAllocateFn fn_alloc,
    CiniFreeFn fn_free,
    void *allocator
) {
    CiniArena *arena = fn_alloc(
        cini_max_i64(64, sizeof(CiniArena))
      + capacity, allocator
    );
    arena->allocation =
        ((uint8_t *)arena)
      + cini_max_i64(64, sizeof(CiniArena));
    arena->capacity = capacity;
    arena->usage = 0;
    arena->fn_allocate = fn_alloc;
    arena->fn_free = fn_free;
    arena->allocator = allocator;
    arena->continuation = NULL;
    return arena;
}

void cini_free_arena(
    CiniArena *arena
) {
    if (arena->continuation)
    {
        cini_free_arena(arena->continuation);
    }
    arena->fn_free(arena, arena->allocator);
}

void * cini_arena_alloc(
    CiniArena *arena,
    uint32_t amount
) {
    if ((arena->usage + amount) >= arena->capacity)
    {
        if ( ! arena->continuation)
        {
            // Allocate a new arena that is double the size of
            // the current one, but *at least* double the size
            // of the allocation that is currently being made.
            arena->continuation = cini_new_arena(
                cini_max_i64(
                    arena->usage * 2,
                    amount * 2),
                arena->fn_allocate,
                arena->fn_free,
                arena->allocator
            );
        }
        return cini_arena_alloc(
            arena->continuation,
            amount);
    }
    void *allocation = &((uint8_t *)arena->allocation)[arena->usage];
    arena->usage += amount;
    return allocation;
}

char * cini_arena_copy_string(CiniArena *arena, const char *string)
{
    uint32_t len_string = strlen(string);
    char *string_copy = cini_arena_alloc(arena, len_string + 1);
    memcpy(string_copy, string, len_string + 1);
    return string_copy;
}



// ==> UTF-8 stream character extraction

int32_t cini_distance_to_last_utf8_rune_start(
    const char *string,
    int32_t offset
) {
    int32_t bytes_walked = 0;
    while ((offset - bytes_walked) >= 0)
    {
        if (bytes_walked > 4)
        {
            return -1;
        }
        if ((string[offset - bytes_walked] >> 6) != 0x02)
        {
            return bytes_walked;
        }
        ++bytes_walked;
    }
    return -2;
}

int32_t cini_identify_utf8_rune_length(
    const char *string,
    uint32_t offset
) {
    char head_byte = string[offset];
    // If this is ASCII

    if ((head_byte & (1 << 7)) == 0)
    {
        return 1;
    }
    // UTF-8 - only

    uint32_t length = 0;
    while (length < 5)
    {
        head_byte <<= 1;
        if ((head_byte & (1 << 7)) == 0)
        {
            break;
        }
        ++length;
    }
    if (length > 4)
    {
        return -1;
    }
    if (length < 2)
    {
        return -2;
    }
    return length;
}

uint32_t cini_postprocess_utf8_head_byte(
    char byte,
    uint32_t num_bytes
) {
    switch (num_bytes)
    {
        case 1: return byte;
        case 2: return byte & 0xe0;
        case 3: return byte & 0xf0;
        case 4: return byte & 0xf8;
    }
    return 0;
}

uint32_t cini_postprocess_utf8_bytes(
    const char *bytes,
    uint32_t num_bytes
) {
    uint32_t result = cini_postprocess_utf8_head_byte(bytes[0], num_bytes);

    uint32_t byte_index = 1;
    while (byte_index < num_bytes)
    {
        result <<= 6;
        result |= bytes[byte_index] & 0x3f;
        ++byte_index;
    }
    return result;
}

uint_least32_t cini_extract_utf8(
    const char *string,
    uint_fast32_t offset,
    uint_fast32_t *remaining
) {
    if (string[offset] == 0x00)
    {
        return 0;
    }
    int32_t offset_into_rune = cini_distance_to_last_utf8_rune_start(string, offset);
    if (offset_into_rune < 0)
    {
        return 0;
    }
    offset -= offset_into_rune;

    int32_t rune_length = cini_identify_utf8_rune_length(string, offset);
    if (rune_length < 0)
    {
        return 0;
    }
    if (remaining)
    {
        *remaining = rune_length - offset_into_rune;
    }
    return cini_postprocess_utf8_bytes(&string[offset], rune_length);
}



// ==> ASCII range checks

bool cini_check_newline(const char *string, uint_fast32_t offset, uint_fast32_t *next)
{
    uint_fast32_t subject = cini_extract_utf8(string, offset, &offset);
    if (subject == '\r')
    {
        uint_fast32_t offset_backup = offset;
        if (cini_extract_utf8(string, offset, NULL) != '\n')
            (*next) = offset_backup;
        return true;
    }
    if (subject == '\n')
    {
        (*next) = offset;
        return true;
    }
    return false;
}

bool cini_is_lowercase(uint32_t rune)
{
    if (rune < 'a')
    {
        return false;
    }
    if (rune > 'z')
    {
        return false;
    }
    return true;
}

bool cini_is_uppercase(uint32_t rune)
{
    if (rune < 'A')
    {
        return false;
    }
    if (rune > 'Z')
    {
        return false;
    }
    return true;
}

bool cini_is_letter(uint32_t rune)
{
    if (cini_is_lowercase(rune))
    {
        return true;
    }
    if (cini_is_uppercase(rune))
    {
        return true;
    }
    return false;
}

bool cini_is_digit(uint32_t rune)
{
    if (rune < '0')
    {
        return false;
    }
    if (rune > '9')
    {
        return false;
    }
    return true;
}

bool cini_rune_is_in_ascii_special_block_1(uint32_t rune)
{
    if (rune < '!')
    {
        return false;
    }
    if (rune > '/')
    {
        return false;
    }
    return true;
}

bool cini_rune_is_in_ascii_special_block_2(uint32_t rune)
{
    if (rune < ':')
    {
        return false;
    }
    if (rune > '@')
    {
        return false;
    }
    return true;
}

bool cini_rune_is_in_ascii_special_block_3(uint32_t rune)
{
    if (rune < '[')
    {
        return false;
    }
    if (rune > '`')
    {
        return false;
    }
    return true;
}

bool cini_rune_is_in_ascii_special_block_4(uint32_t rune)
{
    if (rune < '{')
    {
        return false;
    }
    if (rune > '~')
    {
        return false;
    }
    return true;
}

bool cini_is_sign(uint32_t rune)
{
    if (cini_rune_is_in_ascii_special_block_1(rune))
    {
        return true;
    }
    if (cini_rune_is_in_ascii_special_block_2(rune))
    {
        return true;
    }
    if (cini_rune_is_in_ascii_special_block_3(rune))
    {
        return true;
    }
    if (cini_rune_is_in_ascii_special_block_4(rune))
    {
        return true;
    }
    return false;
}

bool cini_is_whitespace(
    uint_least32_t rune
) {
    if (rune == ' ')
    {
        return true;
    }
    if (rune == '\t')
    {
        return true;
    }
    return false
}

uint_fast32_t cini_count_repetitions(
    const char *string,
    uint_fast32_t len_string,
    uint_least32_t base_character
) {
    uint_fast32_t num_repetitions = 0;
    uint_fast32_t offset = 0;
    while (offset < len_string)
    {
        uint_fast32_t len_character;
        uint_least32_t character = cini_extract_utf8(
            string,
            offset,
            &len_character
        );
        if (character != base_character)
        {
            break;
        }
        ++num_repetitions;
        ++len_character;
    }
    return num_repetitions;
}

CiniAsciiSign cini_rune_to_sign_enum(uint32_t rune)
{
    switch (rune)
    {
        case '!':   return CINI_ASCII_EXCLAMATION_MARK;
        case '"':   return CINI_ASCII_DOUBLE_QUOTATION_MARK;
        case '#':   return CINI_ASCII_HASH_SIGN;
        case '$':   return CINI_ASCII_DOLLAR_SIGN;
        case '%':   return CINI_ASCII_PERCENT_SIGN;
        case '&':   return CINI_ASCII_AMPERSAND;
        case '\'':  return CINI_ASCII_SINGLE_QUOTATION_MARK;
        case '(':   return CINI_ASCII_OPENING_PARENTHESIS;
        case ')':   return CINI_ASCII_CLOSING_PARENTHESIS;
        case '*':   return CINI_ASCII_ASTERISK;
        case '+':   return CINI_ASCII_PLUS;
        case ',':   return CINI_ASCII_COMMA;
        case '-':   return CINI_ASCII_MINUS;
        case '.':   return CINI_ASCII_POINT;
        case '/':   return CINI_ASCII_SLASH;
        case ':':   return CINI_ASCII_COLON;
        case ';':   return CINI_ASCII_SEMICOLON;
        case '<':   return CINI_ASCII_SMALLER_THAN;
        case '=':   return CINI_ASCII_EQUALS_SIGN;
        case '>':   return CINI_ASCII_BIGGER_THAN;
        case '?':   return CINI_ASCII_QUESTION_MARK;
        case '@':   return CINI_ASCII_AT_SIGN;
        case '[':   return CINI_ASCII_OPENING_SQUARE_BRACKET;
        case '\\':  return CINI_ASCII_BACKSLASH;
        case ']':   return CINI_ASCII_CLOSING_SQUARE_BRACKET;
        case '^':   return CINI_ASCII_CIRCUMFLEX;
        case '_':   return CINI_ASCII_UNDERSCORE;
        case '`':   return CINI_ASCII_TICK;
        case '{':   return CINI_ASCII_OPENING_CURLY_BRACE;
        case '|':   return CINI_ASCII_VERTICAL_BAR;
        case '}':   return CINI_ASCII_CLOSING_CURLY_BRACE;
        case '~':   return CINI_ASCII_TILDE;
    }
    return CINI_ASCII_NOT_A_SIGN;
}

