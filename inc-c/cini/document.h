
#ifndef CINI_DOCUMENT_H
#define CINI_DOCUMENT_H

#include <stdint.h>

#include <cini/enumerations.h>
#include <cini/utility.h>

typedef struct CiniDocument CiniDocument;
typedef struct CiniSection CiniSection;
typedef struct CiniField CiniField;

typedef enum
{
    CINI_UNKNOWN_VALUE = 0,

    CINI_VALUE_INTEGER = 1,
    CINI_VALUE_DECIMAL = 1 << 1,
    CINI_VALUE_STRING = 1 << 2,
    CINI_VALUE_BOOLEAN = 1 << 3,
    CINI_VALUE_ARRAY = 1 << 4,

    CINI_INVALID_VALUE = 0xffff

} CiniValueType;

struct CiniField
{
    CiniField *next;

    uint_fast16_t applicable_types;
    uint_fast16_t len_key;
    uint_fast32_t len_value;

    char *key;
    char *value;
};

struct CiniSection
{
    CiniSection *next;
    
    char *name;
    CiniField *first_field;
};

struct CiniDocument
{
    uint_fast32_t num_sections;
    uint_fast32_t num_values;
    CiniSection *first_section;
    CiniSection *root_section;

    CiniAllocateFn fn_alloc;
    CiniFreeFn fn_free;
    void *allocator;

    CiniArena *arena;
};

CiniDocument * cini_malloc_document();

CiniDocument * cini_new_document(
    CiniAllocateFn fn_alloc,
    CiniFreeFn fn_free,
    void *userdata
);

void cini_free_document(
    CiniDocument *document
);

/// @todo This isn't implemented (yet)
void cini_reset_document(
    CiniDocument *document
);

/// @todo This isn't implemented (yet)
void cini_set_feature(
    CiniDocument *document,
    CiniFeature feature,
    bool value
);

/// @todo This isn't implemented (yet)
const char * cini_get_log(
    CiniDocument *document
);

/// @todo This isn't implemented (yet)
int_fast32_t cini_get_log_entry(
    CiniDocument *document,
    uint_fast32_t index,
    uint_fast32_t 
);

#endif // CINI_DOCUMENT_H

