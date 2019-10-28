#pragma once
#include"pch.h"

typedef enum {
    JSON_NULL, JSON_FALSE, JSON_TRUE, JSON_NUMBER, JSON_STRING, 
    JSON_ARRAY, JSON_OBJECT, JSON_UNRESOLVED
}nfjson_type;

enum {
    NFJSON_PARSE_OK = 0,
    NFJSON_PARSE_EXPECT_VALUE,
    NFJSON_PARSE_INVALID_VALUE,
    NFJSON_PARSE_ROOT_NOT_SINGULAR,
    NFJSON_PARSE_NUMBER_TOO_BIG,
    NFJSON_PARSE_MISS_QUOTATION_MARK,
    NFJSON_PARSE_INVALID_STRING_ESCAPE,
    NFJSON_PARSE_INVALID_STRING_CHAR,
};

typedef struct {
    union {
        struct { char *s; size_t len; }s;/* type == JSON_STRING */
        double n;/* type == JSON_NUMBER */
    }u;
    nfjson_type type;
}nfjson_value;/* may using C11 grammar like v->s for  v->u.s.s */

typedef struct {
    const char *json;/*the parsing position in the json*/
    char *stack;/*parsing buffer*/
    size_t size;/*size of stack*/
    size_t top;/*pointer of stack*/
}nfjson_context;
