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

size_t nfjson_get_array_size(const nfjson_value * val);

nfjson_value * nfjson_get_array_element(const nfjson_value * val, size_t index);

size_t nfjson_get_object_size(nfjson_value * val);

int nfjson_object_contains(nfjson_value * val, nfjson_string * key);

nfjson_value * nfjson_get_object_value(nfjson_value * val, nfjson_string * key);

size_t nfjson_get_object_key(nfjson_value *val, const nfjson_string **_keys);
