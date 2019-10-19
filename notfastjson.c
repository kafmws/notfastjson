#include "pch.h"
#include "notfastjson.h"

/**
*   ws = *(
*       %x20 /      ; Space
*       %x09 /      ; Horizontal tab
*       %x0A /      ; Line feed or New line
*       %x0D)       ;  Carriage return
**/    
static void nfjson_parse_whitespace(nfjson_context *c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++;
    c->json = p;
}

/* null = "null" */
static int nfjson_parse_null(nfjson_context *c, nfjson_value *val) {
    if (c->json[0] == 'n' && c->json[1] == 'u' && c->json[2] == 'l' && c->json[3] == 'l') {
        c->json += 4;
        val->type = JSON_NULL;
        return NFJSON_PARSE_OK;
    }
    return NFJSON_PARSE_INVALID_VALUE;
}

/* true = "true" */
static int nfjson_parse_true(nfjson_context *c, nfjson_value *val) {
    if (c->json[0] == 't' && c->json[1] == 'r' && c->json[2] == 'u' && c->json[3] == 'e') {
        c->json += 4;
        val->type = JSON_TRUE;
        return NFJSON_PARSE_OK;
    }
    return NFJSON_PARSE_INVALID_VALUE;
}

/* false = "false" */
static int nfjson_parse_false(nfjson_context *c, nfjson_value *val) {
    if (c->json[0] == 'f' && c->json[1] == 'a' && c->json[2] == 'l' && c->json[3] == 's' && c->json[4] == 'e') {
        c->json += 5;
        val->type = JSON_FALSE;
        return NFJSON_PARSE_OK;
    }
    return NFJSON_PARSE_INVALID_VALUE;
}

/* value = null / false / true */
static int nfjson_parse_value(nfjson_context *c, nfjson_value *val) {
    switch (*(c->json)) {
    case 'n': return nfjson_parse_null(c, val);
    case 't': return nfjson_parse_true(c, val);
    case 'f': return nfjson_parse_false(c, val);
    case '\0': return NFJSON_PARSE_EXPECT_VALUE;
    default:  return NFJSON_PARSE_INVALID_VALUE;
    }
}

int nfjson_parse(nfjson_value *val, const char *json) {
    assert(NULL != val);
    nfjson_context context;
    context.json = json;
    val->type = JSON_UNRESOLVED;
    nfjson_parse_whitespace(&context);
    int prase_status = nfjson_parse_value(&context, val);
    if (prase_status == NFJSON_PARSE_OK){
        nfjson_parse_whitespace(&context);
        if(*(context.json)) return NFJSON_PARSE_ROOT_NOT_SINGULAR;
    }
    else return prase_status;
}

nfjson_type nfjson_get_type(const nfjson_value *val){
    return val->type;
}