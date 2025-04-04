
#ifndef CINI_PARSER_H
#define CINI_PARSER_H

#include <stdio.h>
#include <stdint.h>

#include <cini/enumerations.h>
#include <cini/document.h>

int_fast8_t cini_parse_source(
    CiniDocument *buffer,
    const char *source
);

int_fast8_t cini_parse_source_limited(
    CiniDocument *buffer,
    const char *source,
    uint_fast32_t len_source
);

int_fast8_t cini_parse_from_path(
    CiniDocument *buffer,
    const char *path
);

int_fast8_t cini_parse_file_pointer(
    CiniDocument *buffer,
    FILE *pointer
);

#endif // CINI_PARSER_H

