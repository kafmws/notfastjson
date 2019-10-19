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
};

typedef struct {
    nfjson_type type;
}nfjson_value;

typedef struct {
    const char *json;/*the parsing position in the json*/
}nfjson_context;

int nfjson_parse(nfjson_value *val, const char *json);

nfjson_type nfjson_get_type(const nfjson_value *val);