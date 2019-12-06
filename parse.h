#include"pch.h"
#include"notfastjson.h"

int nfjson_parse(nfjson_value *val, const char *json);

char * nfjson_stringify(nfjson_value * val, size_t * _len, int * status);
