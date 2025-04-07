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
        puts("Syntax Error: Empty section header.");
        parser->status = CINI_SYNTAX_ERROR;
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
    uint_fast32_t name_start = offset;
    
    // Jump over the opening square bracket

    uint_fast32_t len_character;
    uint_least32_t character = cini_extract_utf8(
        parser->source,
        offset,
        &len_character
    );
    offset += len_character;

    while (offset < parser->len_source)
    {
        character = cini_extract_utf8(
            parser->source,
            offset,
            &len_character
        );
        if (character == '\n')
        {
            /// @todo Find the next syntactically correct thing and
            ///       continue parsing there, but keep the status.

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

uint_fast32_t cini_internal_parse_field(
    struct CiniParser *parser,
    uint_fast32_t offset,
    CiniSection *active_section
) {
    uint_fast32_t start_offset = offset;

    // Jump over all possible whitespaces in front of the key

    uint_fast32_t len_character;
    uint_least32_t character;
    
    while (offset < parser->len_source)
    {
        character = cini_extract_utf8(
            parser->source,
            offset,
            &len_character
        );
        if ( ! cini_is_whitespace(character))
        {
            break;
        }
        ++len_character;
    }

    // Find equals sign

    uint_fast32_t key_start = offset;
    while (offset < parser->len_source)
    {
        character = cini_extract_utf8(
            parser->source,
            offset,
            &len_character
        );
        if (cini_is_whitespace(character))
        {
            break;
        }
        offset += len_character;
    }
    if (parser->source[offset] == '=')
    {
        return offset;
    }
    uint_fast32_t equals_position = offset;

    // Go back to the last character of the key
    
    while (offset < parser->len_source)
    {
        character = cini_extract_utf8(
            parser->source,
            offset,
            &len_character
        );
        if (cini_is_whitespace(character))
        {
            break;
        }
        offset += len_character;
    }

    CiniField field;
    field.len_key = offset - key_start;
    field.key = cini_arena_alloc(
        parser->document->arena,
        field.len_key + 1
    );
    memcpy(
        field.len_key,
        &parser->source[key_start],
        field.len_key
    );
    field.key[field.len_key] = 0;

    return 0;
}

CiniSection * cini_internal_find_sub_section(
    CiniDocument *document,
    CiniSection *section,
    const char *sub_section_name
) {
    uint_fast32_t sub_section_index = 0;
    while (sub_section_index < section->num_sub_sections)
    {
        if (
            ! strcmp(
                section->sub_sections[sub_section_index]->name,
                sub_section_name
        )) {
            return section->sub_sections[sub_section_index];
        }
        ++sub_section_index;
    }
    return NULL;
}

CiniSection * cini_internal_find_section(
    CiniDocument *document,
    const char **path
) {
    // The section currently deepest into the
    // hierarchy that matches the query.
    CiniSection *section = document->root_section;
    uint_fast32_t path_element_index = 0;
    while (path[path_element_index])
    {
        section = cini_internal_find_sub_section(
            document,
            section,
            path[path_element_index]
        );
        if ( ! section)
        {
            break;
        }
        ++path_element_index;
    }
    return section;
}

CiniSection * cini_internal_add_sub_section(
    CiniDocument *document,
    CiniSection *section,
    const char *name
) {
    if(section->sub_sections == NULL)
    {
        section->sub_sections_capacity = 4;
        section->sub_sections = cini_arena_alloc(
            document->arena,
            section->sub_sections_capacity
          * sizeof(CiniSection *)
        );
    }
    if (
        section->num_sub_sections
      >= section->sub_sections_capacity
    ) {
        section->sub_sections_capacity *= 2;
        CiniSection **resized_sub_sections = cini_arena_alloc(
            document->arena,
            section->sub_sections_capacity * sizeof(CiniSection *)
        );
        memcpy(
            resized_sub_sections,
            section->sub_sections,
            section->num_sub_sections * sizeof(CiniSection *)
        );
        section->sub_sections = resized_sub_sections;
    }
    CiniSection *sub_section = cini_arena_alloc(
        document->arena,
        sizeof(CiniSection)
    );
    sub_section->name = cini_arena_copy_string(
        document->arena,
        name
    );
    memset(sub_section, 0, sizeof(CiniSection));
    section->sub_sections[section->num_sub_sections] = sub_section;
    ++section->num_sub_sections;
    return sub_section;
}

CiniSection * cini_internal_force_create_section(
    CiniDocument *document,
    const char **path
) {
    // The section currently deepest into the
    // hierarchy that matches the query.
    CiniSection *section = document->root_section;
    uint_fast32_t path_element_index = 0;
    while (path[path_element_index])
    {
        CiniSection *sub_section = cini_internal_find_sub_section(
            document,
            section,
            path[path_element_index]
        );
        if ( ! sub_section)
        {
            break;
        }
        section = sub_section;
        ++path_element_index;
    }
    while (path[path_element_index])
    {
        CiniSection *sub_section = cini_internal_add_sub_section(
            document,
            section,
            path[path_element_index]
        );
        if ( ! sub_section)
        {
            /// @todo This indicates an allocation failure.
        }
        section = sub_section;
        ++path_element_index;
    }
    return section;
}

CiniSection * cini_internal_find_or_create_section(
    CiniDocument *document,
    const char **path
) {
    CiniSection *section = cini_internal_find_section(
        document,
        path
    );
    if (section)
    {
        return section;
    }
    return cini_internal_force_create_section(
        document,
        path
    );
}

int_fast8_t cini_parse_source_limited(
    CiniDocument *buffer,
    const char *source,
    uint_fast32_t len_source
) {
    if (buffer->root_section == NULL)
    {
        puts("Not Initialized: Documents must be initialized before parsing.");
        return CINI_NOT_INITIALIZED;
    }

    struct CiniParser parser;
    parser.document = buffer;
    parser.status = CINI_SUCCESS;
    parser.source = source;
    parser.len_source = len_source;

    CiniSection *current_section = parser.document->root_section;

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
            char **section_path = NULL;

            // 'status' contains the number of section path elements
            // OR zero, if the parsing process failed there.
            uint_fast32_t status = cini_internal_parse_section_header(
                &parser,
                offset,
                &section_path
            );
            if (status == 0)
            {
                break;
            }
            offset += status;
            CiniSection *section = cini_internal_find_or_create_section(
                parser.document,
                (const char **) section_path
            );
            current_section = section;
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
    // Pass full source to cini_parse_source_limited

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

