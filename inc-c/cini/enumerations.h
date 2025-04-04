
#ifndef CINI_STATUS_H
#define CINI_STATUS_H

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

#endif // CINI_STATUS_H

