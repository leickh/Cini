#include <cini/document.h>

#include <stddef.h>
#include <stdlib.h>

void * cini_call_wrapped_malloc(
    uint_fast32_t amount,
    void *userdata
) {
    userdata = userdata; // To avoid Unused Parameter - warnings 
    return malloc(amount);
}

void cini_call_wrapped_free(
    void *pointer,
    void *userdata
) {
    userdata = userdata; // To avoid Unused Parameter - warnings 
    free(pointer);
}



CiniDocument * cini_malloc_document()
{
    return cini_new_document(
        cini_call_wrapped_malloc,
        cini_call_wrapped_free,
        NULL
    );
}

CiniDocument * cini_new_document(
    CiniAllocateFn fn_alloc,
    CiniFreeFn fn_free,
    void *userdata
) {
    CiniDocument *document = fn_alloc(
        sizeof(CiniDocument),
        userdata
    );
    document->arena = cini_new_arena(
        16384,
        fn_alloc,
        fn_free,
        userdata
    );
    document->fn_alloc = fn_alloc;
    document->fn_free = fn_free;
    document->num_sections = 1;
    document->first_section = cini_arena_alloc(
        document->arena,
        sizeof(CiniSection)
    );
    document->root_section = document->first_section;
    document->root_section->name = "$";
    document->root_section->first_field = NULL;
    document->root_section->linear_next = NULL;
    document->root_section->sub_sections_capacity = 0;
    document->root_section->num_sub_sections = 0;
    document->root_section->sub_sections = NULL;

    return document;
}

void cini_free_document(
    CiniDocument *document
) {
    cini_free_arena(document->arena);
    document->fn_free(document, document->allocator);
}

