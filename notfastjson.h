#pragma once
#include"pch.h"
#include"hash_table.h"

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
    NFJSON_PARSE_INVALID_UNICODE_HEX,
    NFJSON_PARSE_INVALID_UNICODE_SURROGATE,
    NFJSON_EXTRA_COMMA,
    NFJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
    NFJSON_PARSE_MISS_KEY,
    NFJSON_PARSE_MISS_COLON,
    NFJSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET,
};

typedef struct nfjson_value nfjson_value;

typedef struct { char *s; size_t len; } nfjson_string;

struct nfjson_value {
    union {
        nfjson_string s;/* type == JSON_STRING */
        struct { nfjson_value *e; size_t len; }a;/* type == JSON_ARRAY */
        hash_table *hto;/* type == JSON_OBJECT */
        double n;/* type == JSON_NUMBER */
    }u;
    nfjson_type type;
};/* may using C11 grammar like v->s for  v->u.s.s */

typedef struct {
    const char *json;/*the parsing position in the json*/
    char *stack;/*parsing buffer*/
    size_t size;/*size of stack*/
    size_t top;/*pointer of stack*/
}nfjson_context;
