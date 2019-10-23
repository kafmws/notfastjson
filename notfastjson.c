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

/* null = "null" true = "true" false = "false" */
static int nfjson_parse_literal(nfjson_context *c, nfjson_value *val, const char *literal, int json_type) {
    const char *json = c->json;
    while (*literal && *literal == *json) {
        literal++; json++;
    }
    if (*literal)return NFJSON_PARSE_INVALID_VALUE;
    c->json = json;
    val->type = json_type;
    return NFJSON_PARSE_OK;
}

#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')
/**
*   number = ["-"] int[frac][exp]
*   int = "0" / digit1 - 9 * digit
*   frac = "." 1 * digit
*   exp = ("e" / "E")["-" / "+"] 1 * digit
**/
static int nfjson_parse_number(nfjson_context *c, nfjson_value *val) {
    int parse_type = -1;
    const char *test = c->json;
    errno = 0;
    if (*test == '-')test++;
    if (ISDIGIT(*test)) {
        if (*test == '0') {
            test++; 
            if (*test == '.') {
                test++;
                if (!(ISDIGIT(*test)))return NFJSON_PARSE_INVALID_VALUE;//0.
                while (ISDIGIT(*test))test++;
            }
            else if (*test != 'e'&& *test != 'E' && *test != '\0') { parse_type = NFJSON_PARSE_ROOT_NOT_SINGULAR; goto out; }//parse mostly valid text
        }
        else {
            while (ISDIGIT(*test)) test++;
            if (*test == '.') {
                test++;
                if (!(ISDIGIT(*test)))return NFJSON_PARSE_INVALID_VALUE;//num+.
                while (ISDIGIT(*test))test++;
            }
        }
        if (*test == 'e' || *test == 'E') {
            test++;
            if (*test == '+' || *test == '-')test++;
            if (!(ISDIGIT(*test))) { parse_type = NFJSON_PARSE_ROOT_NOT_SINGULAR; goto out; }
            while (ISDIGIT(*test))test++;
        }
    } else return NFJSON_PARSE_INVALID_VALUE;
out:
    if (parse_type == NFJSON_PARSE_ROOT_NOT_SINGULAR) {
        int len = test - (c->json);
        char *num = malloc(sizeof(char)*(len + 1));
        memcpy(num, c->json, len);
        num[len] = 0;
        val->n = strtod(num, NULL);
        free(num);
        /*char *mod = test;
        char ch = *mod;
        printf("%s\n", c->json);
        *mod = 0;
        val->n = strtod(c->json, NULL);
        *mod = ch;*/
    }
    else val->n = strtod(c->json, NULL);
    if (errno == ERANGE && (val->n == HUGE_VAL || val->n == -HUGE_VAL)) { return NFJSON_PARSE_NUMBER_TOO_BIG; }
    c->json = test;
    val->type = JSON_NUMBER;
    return parse_type > 0 ? parse_type : NFJSON_PARSE_OK;
}

/* value = null / false / true / number */
static int nfjson_parse_value(nfjson_context *c, nfjson_value *val) {
    switch (*(c->json)) {
    case 'n': return nfjson_parse_literal(c, val, "null", JSON_NULL);
    case 't': return nfjson_parse_literal(c, val, "true", JSON_TRUE);
    case 'f': return nfjson_parse_literal(c, val, "false", JSON_FALSE);
    case '0':case '1':case '2':case '3':case '4':case '5':case '6':
    case '7':case '8':case '9':case '-':
        return nfjson_parse_number(c, val);
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
    int parse_status = nfjson_parse_value(&context, val);
    if (parse_status == NFJSON_PARSE_OK){
        nfjson_parse_whitespace(&context);
        if(*(context.json)) return NFJSON_PARSE_ROOT_NOT_SINGULAR;
    }
    return parse_status;
}

nfjson_type nfjson_get_type(const nfjson_value *val){
    return val->type;
}

double nfjson_get_number(const nfjson_value *val) {
    assert(val && val->type == JSON_NUMBER);
    return val->n;
}