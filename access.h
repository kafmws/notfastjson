#include"pch.h"
#include"notfastjson.h"

nfjson_type nfjson_get_type(const nfjson_value *val);

void nfjson_set_null(nfjson_value * val);

int nfjson_get_boolean(const nfjson_value * val);

void nfjson_set_boolean(nfjson_value * val, int b);

double nfjson_get_number(const nfjson_value * val);

void nfjson_set_number(nfjson_value * val, double n);

const char * nfjson_get_string(const nfjson_value * val);

size_t nfjson_get_string_length(const nfjson_value * val);

void nfjson_set_string(nfjson_value * val, const char * s, size_t len);
