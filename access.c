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

size_t nfjson_get_object_size(nfjson_value *val) {
    assert(val && val->type == JSON_OBJECT);
    return val->u.ht->cnt;
}

int nfjson_object_contains(nfjson_value *val, nfjson_string *key) {
    assert(val && val->type == JSON_OBJECT);
    return hash_table_get(val->u.ht, (void *)key) != NULL;
}

size_t nfjson_get_object_key(nfjson_value *val,nfjson_string *_keys[]) {
    assert(val->type == JSON_OBJECT);
    nfjson_string **keys = (nfjson_string **)_keys;
    size_t size = (size_t)val->u.ht->cnt, i = 0, cnt = 0;
    kv **table = val->u.ht->table;
    kv* kv_list = NULL;
    while (cnt < size) {
        if (kv_list = table[i]) {
            while (kv_list) {
                keys[cnt++] = (nfjson_string *)kv_list->key;
                kv_list = kv_list->next;
            }
        }
        i++;
    }
    return cnt;
}

nfjson_value *nfjson_get_object_value(nfjson_value *val, nfjson_string *key) {
    assert(val && val->type == JSON_OBJECT);
    return hash_table_get(val->u.ht, (void *)key);
}