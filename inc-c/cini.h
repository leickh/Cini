
#ifndef CINI_H
#define CINI_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef void CiniDocument;

typedef enum
{
    // ==> Input-caused errors

    CINI_INVALID_POINTER = -127,
    CINI_INVALID_ENUM_VALUE,
    CINI_FILE_NOT_FOUND,
    CINI_SECTION_NONEXISTENT,
    CINI_KEY_NONEXISTENT,
    CINI_SYNTAX_ERROR,
    
    // ==> Internal Errors

    CINI_GENERIC_INTERNAL_ERROR = -16,
    CINI_LIMITATION_EXCEEDED,
    CINI_ALLOCATION_FAILURE,
    CINI_NOT_INITIALIZED,

    CINI_SUCCESS = 0,

} CiniStatus;

typedef enum
{
    CINI_FEATURE_CUSTOM_ALLOCATOR = 1,
    CINI_FEATURE_LOG = 1 << 1

} CiniFeature;

typedef void * (*CiniAllocateFn)(
    uint_fast32_t amount,
    void *userdata
);

typedef void (*CiniFreeFn)(
    void *pointer,
    void *userdata
);



// ==> Document Management

CiniDocument * cini_malloc_document();

CiniDocument * cini_new_document(
    CiniAllocateFn fn_alloc,
    CiniFreeFn fn_free,
    void *userdata
);

void cini_free_document(
    CiniDocument *document
);

void cini_reset_document(
    CiniDocument *document
);

void cini_set_allocator(
    CiniDocument *document,
    CiniAllocateFn fn_allocate,
    CiniFreeFn fn_free,
    void *userdata
);

void cini_set_feature(
    CiniDocument *document,
    CiniFeature feature,
    bool value
);

const char * cini_get_log(
    CiniDocument *document
);

int_fast32_t cini_get_log_entry(
    CiniDocument *document,
    uint_fast32_t index,
    uint_fast32_t 
);



// ==> Source Parsing

int_fast8_t cini_parse_source(
    CiniDocument *buffer,
    const char *source
);

int_fast8_t cini_parse_source_part(
    CiniDocument *buffer,
    const char *source_start,
    uint_fast32_t len_part
);

int_fast8_t cini_parse_from_path(
    CiniDocument *buffer,
    const char *path
);

int_fast8_t cini_parse_file_pointer(
    CiniDocument *buffer,
    FILE *pointer
);



// ==> Section Topology

/// @brief Get number of sections within a document or number of
///        sub-sections within another section.
/// @param document
///        Document of which to get the number of sections.
/// @param super_section
///        Superordinate section of which to get the number of
///        sub-sections or `NULL` to get the total number of sections.
/// @return
/// Section count or `CINI_SECTION_NONEXISTENT` if the `super_section`
/// could not be found.
int_fast32_t cini_get_section_count(
    CiniDocument *document,
    const char *super_section
);

/// @brief Get name of section at an index in the list of documents,
///        possibly within a `super_section`.
/// @param document
///        Document of which to get an entry of the section list.
/// @param super_section
///        Superordinate section of which to get the section at an
///        index or `NULL` to access the document's sections linearly.
/// @param index
///        Index of the section.
/// @return 
/// A string that contains the full name of the section (with all
/// superordinate parts). Changing this isn't recommended.
const char * cini_get_section_name(
    CiniDocument *document,
    const char *super_section,
    uint_fast32_t index
);



// ==> Value Gathering

int_fast8_t cini_get_bool(
    CiniDocument *document,
    const char *query,
    bool *buffer
);

int_fast8_t cini_get_int(
    CiniDocument *document,
    const char *query,
    int64_t *buffer
);

int_fast8_t cini_get_decimal(
    CiniDocument *document,
    const char *query,
    double *buffer
);

const char * cini_get_text(
    CiniDocument *document,
    const char *query
);

/// @brief Duplicate a text value from an INI-document into a buffer.
/// @param document
///        Document supposed to contain the value.
/// @param query
///        `<section>:<key>` value to search in the document.
/// @param buffer
///        Buffer into which to write the text, or NULL for getting
///        the stored text's length.
/// @param len_buffer
///        Length of `buffer`, or -1 to ignore
/// @return
/// One of three possible states:  
/// 
/// - Negative on error  
/// 
/// - Zero on success  
/// 
/// - Stored text's length on success if `buffer` is NULL  
///
int_fast32_t cini_write_text(
    CiniDocument *document,
    const char *query,
    const char *buffer,
    int_fast32_t len_buffer
);

#endif // CINI_H

