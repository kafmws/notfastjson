#include"pch.h"
#include"notfastjson.h"
#include "memory.h"

void nfjson_init(nfjson_value *val) {
    memset(val, 0, sizeof(nfjson_value));
    val->type = JSON_UNRESOLVED;
}

void nfjson_free(nfjson_value *val) {
    assert(val);
    switch (val->type) {
    case JSON_UNRESOLVED:
        return;
    case JSON_STRING:
        if(val->u.s.s)free(val->u.s.s); break;
    case JSON_ARRAY:
    {
        int len = (int)val->u.a.len - 1;
        for (; len >= 0; len--) {
            nfjson_free(val->u.a.e + len);
        }
        if (val->u.a.e) { free(val->u.a.e); }
    }; break;
    case JSON_OBJECT:
        if (val->u.ht) { hash_table_free(val->u.ht); }break;
    default:
        break;
    }
    memset(val, 0, sizeof(nfjson_value));
    val->type = JSON_UNRESOLVED;
}

void nfjson_string_free(nfjson_string *str) {
    assert(str);
    if (str->s) { free(str->s); str->s = NULL; }
    free(str);
}