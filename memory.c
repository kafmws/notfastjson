#include"pch.h"
#include"notfastjson.h"
#include "memory.h"

void nfjson_init(nfjson_value *val) {
    nfjson_free(val);
    memset(val, 0, sizeof(nfjson_value));
    val->type = JSON_UNRESOLVED;
}

void nfjson_free(nfjson_value *val) {
    assert(val);
    if (val->type == JSON_UNRESOLVED)return;
    if (val->type == JSON_STRING) { free(val->u.s.s); val->u.s.s = NULL; }
    else if (val->type == JSON_ARRAY) {
        int len = (int)val->u.a.len - 1;
        for (; len >= 0; len--) {
            nfjson_free(val->u.a.e + len);
        }
        if(val->u.a.e)free(val->u.a.e);
    }
    val->type = JSON_UNRESOLVED;
}