// "hash_table.h"
typedef struct _kv {
    void *key;
    void *val;
    unsigned int hash;
    struct _kv *next;
}kv;

typedef struct {
    kv* *table;
    unsigned int(*hash_func)(void *key);
    int(*cmp_func)(const void *k, const void *key);
    void(*free_key)(void *ptr);
    void(*free_value)(void *ptr);
    int cnt;
    int table_size;
}hash_table;

// "notfastjson.h"
typedef enum {
    JSON_NULL, JSON_FALSE, JSON_TRUE, JSON_NUMBER, JSON_STRING,
    JSON_ARRAY, JSON_OBJECT, JSON_UNRESOLVED
}nfjson_type;

enum {
    NFJSON_PARSE_OK = 0,
    NFJSON_PARSE_EXPECT_VALUE,
    NFJSON_PARSE_INVALID_VALUE,
    NFJSON_PARSE_ROOT_NOT_SINGULAR,
    NFJSON_PARSE_NUMBER_TOO_BIG,
    NFJSON_PARSE_MISS_QUOTATION_MARK,
    NFJSON_PARSE_INVALID_STRING_ESCAPE,
    NFJSON_PARSE_INVALID_STRING_CHAR,
    NFJSON_PARSE_INVALID_UNICODE_HEX,
    NFJSON_PARSE_INVALID_UNICODE_SURROGATE,
    NFJSON_EXTRA_COMMA,
    NFJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
    NFJSON_PARSE_MISS_KEY,
    NFJSON_PARSE_MISS_COLON,
    NFJSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET,
};

typedef struct nfjson_value nfjson_value;

typedef struct { char *s; size_t len; } nfjson_string;

struct nfjson_value {
    union {
        nfjson_string s;/* type == JSON_STRING */
        struct { nfjson_value *e; size_t len; }a;/* type == JSON_ARRAY */
        hash_table *hto;/* type == JSON_OBJECT */
        double n;/* type == JSON_NUMBER */
    }u;
    nfjson_type type;
};/* may using C11 grammar like v->s for  v->u.s.s */

typedef struct {
    const char *json;/*the parsing position in the json*/
    char *stack;/*parsing buffer*/
    size_t size;/*size of stack*/
    size_t top;/*pointer of stack*/
}nfjson_context;

// "parse.h"
int nfjson_parse(nfjson_value *val, const char *json);

// "access.h"
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

// "memory.h"
void nfjson_init(nfjson_value * val);

void nfjson_free(nfjson_value * val);

void nfjson_string_free(nfjson_string * str);