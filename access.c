#include"pch.h"
#include"notfastjson.h"
#include"access.h"
#include"memory.h"

nfjson_type nfjson_get_type(const nfjson_value *val) {
    assert(val);
    return val->type;
}

void nfjson_set_null(nfjson_value *val) {
    assert(val);
    nfjson_free(val);
    val->type = JSON_NULL;
}

int nfjson_get_boolean(const nfjson_value *val) {
    assert(val && (val->type == JSON_FALSE || val->type == JSON_TRUE));
    return val->type == JSON_TRUE;
}

void nfjson_set_boolean(nfjson_value *val, int b) {
    assert(val);
    nfjson_free(val);
    val->type = b ? JSON_TRUE : JSON_FALSE;
}

double nfjson_get_number(const nfjson_value *val) {
    assert(val && val->type == JSON_NUMBER);
    return val->u.n;
}

void nfjson_set_number(nfjson_value *val, double n) {
    assert(val);
    nfjson_free(val);
    val->u.n = n;
    val->type = JSON_NUMBER;
}

const char *nfjson_get_string(const nfjson_value *val) {
    assert(val && val->type == JSON_STRING);
    return val->u.s.s;
}

size_t nfjson_get_string_length(const nfjson_value *val) {
    assert(val && val->type == JSON_STRING);
    return val->u.s.len;
}

void nfjson_set_string(nfjson_value *val, const char *s, size_t len) {
    assert(val && (s || len == 0));//s==0 ""
    nfjson_free(val);
    val->u.s.s = (char *)malloc(sizeof(char)*(len + 1));
    memcpy(val->u.s.s, s, len);
    val->u.s.s[len] = 0;
    val->u.s.len = len;
    val->type = JSON_STRING;
}

size_t nfjson_get_array_size(const nfjson_value *val) {
    assert(val && val->type == JSON_ARRAY);
    return val->u.a.len;
}

nfjson_value *nfjson_get_array_element(const nfjson_value *val, size_t index) {
    assert(val && val->type == JSON_ARRAY);
    if (index < val->u.a.len) return val->u.a.e + index;
    return NULL;
}