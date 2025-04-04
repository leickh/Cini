#include <cini/parser.h>
#include <cini/utility.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct CiniParser
{
    CiniDocument *document;

    uint_fast32_t len_source;
    const char *source;

    CiniStatus status;
};

uint_fast32_t cini_internal_count_section_levels(
    struct CiniParser *parser,
    uint_fast32_t string_start,
    uint_fast32_t len_string
) {
    uint_fast32_t num_levels = 1;
    uint_fast32_t string_offset = 0;
    while (string_offset < len_string)
    {
        uint_fast32_t len_character;
        uint_least32_t character = cini_extract_utf8(
            parser->source,
            string_start + string_offset,
            &len_character
        );
        if (character == '\\')
        {
            /// @todo Parse escape sequences

            string_offset += len_character;
            continue;
        }
        if (character == '"')
        {
            /// @todo Parse string encapsulations

            puts("Limitation Exceeded: String encapsulations aren't supported yet");
            parser->status = CINI_LIMITATION_EXCEEDED;
            return 0;
        }
        if (character == '.')
        {
            ++num_levels;
        }
        if (character == ' ')
        {
            while (string_offset < len_string)
            {
                uint_least32_t character = cini_extract_utf8(
                    parser->source,
                    string_start + string_offset,
                    &len_character
                );
                if (character != ' ')
                {
                    break;
                }
                ++len_character;
            }
            ++num_levels;
        }
        string_offset += len_character;
    }
    return num_levels;
}

/// @brief 
/// @param parser 
/// @param buffer Pointer to an array of strings of section levels.
/// @param string_start 
/// @param len_string 
/// @return 
uint_fast32_t cini_internal_split_section_string(
    struct CiniParser *parser,
    char ***buffer,
    uint_fast32_t string_start,
    uint_fast32_t len_string
) {
    uint_fast32_t num_levels = cini_internal_count_section_levels(
        parser,
        string_start,
        len_string
    );
    if ( ! num_levels)
    {
        return 0;
    }
    *buffer = cini_arena_alloc(
        parser->document->arena,
        sizeof(char *) * (num_levels + 1)
    );
    (*buffer)[num_levels] = NULL;

    uint_fast32_t level_index = 0;
    uint_fast32_t string_offset = 0;
    while (string_offset < len_string)
    {
        uint_fast32_t len_character;
        uint_least32_t character = cini_extract_utf8(
            parser->source,
            string_start + string_offset,
            &len_character
        );
        // Act upon splitters

        if (character == '.')
        {
            ++level_index;
            string_offset += len_character;
            continue;
        }
        if (character == ' ')
        {
            while (string_offset < len_string)
            {
                uint_least32_t character = cini_extract_utf8(
                    parser->source,
                    string_start,
                    &len_character
                );
                if (character != ' ')
                {
                    break;
                }
                ++len_character;
            }
            ++level_index;
            string_offset += len_character;
            continue;
        }
        if (character == '"')
        {
            /// @todo Parse string encapsulated path links

            puts(
                "Limitation Exceeded: "
                "String encapsulations aren't supported yet"
            );
            parser->status = CINI_LIMITATION_EXCEEDED;
            return 0;
        }

        uint_fast32_t path_link_start = string_start + string_offset;
        while (string_offset < len_string)
        {
            character = cini_extract_utf8(
                parser->source,
                string_start + string_offset,
                &len_character
            );
            if (
                 (character == '.')
              || (character == ' ')
              || (character == ']')
            ) {
                break;
            }
            if (character == '\\')
            {
                /// @todo Parse escape sequencess
            }
            string_offset += len_character;
        }
        uint_fast32_t path_link_end = string_start + string_offset;
        uint_fast32_t len_path_link =
              path_link_end - path_link_start;
        char *path_link = cini_arena_alloc(
            parser->document->arena,
            len_path_link + 1
        );
        memcpy(
            path_link,
            &parser->source[path_link_start],
            len_path_link
        );
        path_link[len_path_link] = 0;
        (*buffer)[level_index] = path_link;
        string_offset += len_character;
        ++level_index;
    }
    return num_levels;
}

void cini_internal_write_string_list(
    char **list
) {
    uint_fast32_t index = 0;
    while (list[index])
    {
        printf("%lu: %s\n", index, list[index]);
        ++index;
    };
}

/// @brief Parse the name of a section header.
/// @param parser
///        Parser structure which contains the source, source-length.
/// @param offset
///        Offset into 'parser.source' of the section header's opening
///        square bracket. This doesn't get checked, though.
/// @param section_name
///        Pointer to where to put the split parts of the arena-allocated
///        version of the post-processed and checked section name.
/// @return
/// Zero on failure and the number of bytes until the character right
/// after the section header's closing square bracket on success.
uint_fast32_t cini_internal_parse_section_header(
    struct CiniParser *parser,
    uint_fast32_t offset,
    char ***section_path
) {
    // Jump over the opening square bracket

    uint_fast32_t len_character;
    uint_least32_t character = cini_extract_utf8(
        parser->source,
        offset,
        &len_character
    );
    offset += len_character;

    uint_fast32_t name_start = offset;
    while (offset < parser->len_source)
    {
        character = cini_extract_utf8(
            parser->source,
            offset,
            &len_character
        );
        if (character == '\n')
        {
            puts("Syntax Error: Section header not closed.");
            parser->status = CINI_SYNTAX_ERROR;
            return 0;
        }
        if (character == ']')
        {
            break;
        }
        offset += len_character;
    }
    uint_fast32_t len_raw_string = offset - name_start;

    if (
        cini_internal_split_section_string(
            parser,
            section_path,
            name_start,
            len_raw_string
        ) == 0
    ) {
        return 0;
    }
    return len_raw_string;
}

uint_fast32_t cini_internal_parse_section(
    struct CiniParser *parser,
    uint_fast32_t offset
) {
    char **section_name = NULL;
    uint_fast32_t status = cini_internal_parse_section_header(
        parser,
        offset,
        &section_name
    );
    if ( ! status)
    {
        return 0;
    }
    cini_internal_write_string_list(section_name);
    return status;
}

int_fast8_t cini_parse_source_limited(
    CiniDocument *buffer,
    const char *source,
    uint_fast32_t len_source
) {
    struct CiniParser parser;
    parser.document = buffer;
    parser.status = CINI_SUCCESS;
    parser.source = source;
    parser.len_source = len_source;

    uint_fast32_t offset = 0;
    while (offset < parser.len_source)
    {
        uint_fast32_t len_character;
        uint_least32_t  character = cini_extract_utf8(
            parser.source,
            offset,
            &len_character
        );
        if ( ! len_character)
        {
            parser.status = CINI_GENERIC_INTERNAL_ERROR;
            break;
        }
        if (character == '[')
        {
            int_fast32_t status = cini_internal_parse_section(
                &parser,
                offset
            );
            if (status <= 0)
            {
                break;
            }
            offset += status;
        }
        offset += len_character;
    }
    return parser.status;
}

int_fast8_t cini_parse_source(
    CiniDocument *buffer,
    const char *source
) {
    // Validate arguments

    if (( ! buffer) || ( ! source))
    {
        return CINI_INVALID_POINTER;
    }
    // Pass full source to cini_parse_source_part

    size_t len_source = strlen(source);
    return cini_parse_source_limited(
        buffer,
        source,
        len_source
    );
}

int_fast8_t cini_parse_file_pointer(
    CiniDocument *buffer,
    FILE *pointer
) {
    // Validate arguments

    if (( ! buffer) || ( ! pointer))
    {
        return CINI_INVALID_POINTER;
    }
    if ( ! buffer->arena)
    {
        return CINI_NOT_INITIALIZED;
    }
    // Get the file's length

    fseek(pointer, 0, SEEK_END);
    uint_fast32_t len_file = ftell(pointer);
    fseek(pointer, 0, SEEK_SET);
    
    // Allocate memory for the source

    char *source = cini_arena_alloc(
        buffer->arena,
        len_file + 1
    );

    // Read complete file into buffer

    fread(
        source,
        len_file,
        1,
        pointer
    );
    source[len_file] = 0x00;

    // Pass the source buffer to cini_parse_source

    return cini_parse_source(
        buffer,
        source
    );
}

int_fast8_t cini_parse_from_path(
    CiniDocument *buffer,
    const char *path
) {
    // Validate arguments

    if (( ! buffer) || ( ! path))
    {
        return CINI_INVALID_POINTER;
    }
    FILE *file = fopen(path, "r");
    if ( ! file)
    {
        return CINI_FILE_NOT_FOUND;
    }
    // Pass the file-pointer to cini_parse_file_pointer

    int_fast8_t status = cini_parse_file_pointer(buffer, file);
    fclose(file);
    return status;
}

