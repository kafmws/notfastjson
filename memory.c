#include"pch.h"
#include"notfastjson.h"
#include "memory.h"

void nfjson_init(nfjson_value *val) {
    val->type = JSON_UNRESOLVED;
}

void nfjson_free(nfjson_value *val) {
    assert(val);
    if (val->type == JSON_STRING) free(val->u.s.s);
    val->type = JSON_UNRESOLVED;
}