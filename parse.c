#include "pch.h"
#include "notfastjson.h"
#include"parse.h"
#include"access.h"
#include"memory.h"

#ifndef NFJSON_PARSE_STACK_INIT_SIZE
#define NFJSON_PARSE_STACK_INIT_SIZE 256
#endif
static void *nfjson_context_push(nfjson_context *c, size_t size) {
    assert(c && size > 0);//size == 0?
    if (c->top + size >= c->size) {//extend
        if (c->size == 0) c->size = NFJSON_PARSE_STACK_INIT_SIZE;
        while (c->top + size >= c->size) c->size += c->size >> 1;
        c->stack = (char *)realloc(c->stack, c->size);
    }
    void *re = c->stack + c->top;
    c->top += size;
    return re;
}

#define PUSHC(c, ch) do{ *(char *)(nfjson_context_push(c, sizeof(char))) = (ch); }while(0)

static void *nfjson_context_pop(nfjson_context *c, size_t size) {
    assert(c->top >= size);
    return c->stack + (c->top -= size);
}

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
#define ISHEXCHAR(ch)       ISDIGIT((ch))||((ch)>='A'&&(ch)<='F')||((ch)>='a'&&(ch)<='f')
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
            else if (*test != 'e'&& *test != 'E' && *test != '\0' && *test != ' ' && *test != ',') {// ' ' & ',' for array
                parse_type = NFJSON_PARSE_ROOT_NOT_SINGULAR; goto out; 
            }//parse mostly valid text
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
            if (!(ISDIGIT(*test)) && *test != ' ' && *test != ',') { parse_type = NFJSON_PARSE_ROOT_NOT_SINGULAR; goto out; }
            while (ISDIGIT(*test))test++;
        }
    } else return NFJSON_PARSE_INVALID_VALUE;
out:
    if (parse_type == NFJSON_PARSE_ROOT_NOT_SINGULAR) {
        int len = (int)(test - (c->json));
        char *num = malloc(sizeof(char)*(len + 1));
        memcpy(num, c->json, len);
        num[len] = 0;
        nfjson_set_number(val, strtod(num, NULL));
        free(num);
        /*char *mod = test;
        char ch = *mod;
        printf("%s\n", c->json);
        *mod = 0;
        val->n = strtod(c->json, NULL);
        *mod = ch;*/
    }
    else nfjson_set_number(val, strtod(c->json, NULL));
    if (errno == ERANGE && (val->u.n == HUGE_VAL || val->u.n == -HUGE_VAL)) { nfjson_init(val); return NFJSON_PARSE_NUMBER_TOO_BIG; }
    c->json = test;
    return parse_type > 0 ? parse_type : NFJSON_PARSE_OK;
}

static char *nfjson_unicode_char2dec(const char *str, unsigned int *val) {
    int i = 0;
    char ch;
    unsigned int u = 0;
    do {
        ch = *str++;
        u = u << 4;
        if (ISDIGIT(ch)) { u |= (ch - '0') & 0xFFFF; }
        else if (ch >= 'a'&&ch <= 'f') { u |= (ch - 87) & 0xFFFF; }
        else if (ch >= 'A'&&ch <= 'F') { u |= (ch - 55) & 0xFFFF; }
        else return NULL;
        i++;
    } while (i < 4);
    *val = u;
    return (char *)str;
}

/* codepoint = 0x10000 + (H - 0xD800) × 0x400 + (L - 0xDC00) */
static int nfjson_parse_unicode_char(const char **str, unsigned int *val) {
    unsigned int H = 0, L = 0;
    const char *p = *str; 
    p = nfjson_unicode_char2dec(p, &H);
    if (!p) return NFJSON_PARSE_INVALID_UNICODE_HEX;
    if (0xD800 <= H && H <= 0xDBFF) {
        if(!(p[0] == '\\' && p[1] =='u'))
            return NFJSON_PARSE_INVALID_UNICODE_SURROGATE;
        p += 2;
        p = nfjson_unicode_char2dec(p, &L);
        if (!p || !(0xDC00 <= L && L <= 0xDFFF))
            return NFJSON_PARSE_INVALID_UNICODE_SURROGATE;
        *val = 0x10000 + (H - 0xD800) * 0x400 + (L - 0xDC00);
    }
    else *val = H;
    *str = p;
    return NFJSON_PARSE_OK;
}

/* push codepoint into stack in UTF-8 */
static int nfjson_encode_unicode_codepoint(nfjson_context *c, unsigned int cp) {
    assert(cp >= 0x0 && cp <= 0x10FFFF);
    if (cp < 0x0080) {
        PUSHC(c, (char)cp);
        return 1;
    }
    else if (cp < 0x0800) {
        //0x3F  0011 1111  →  00xx xxxx
        //0x80  1000 0000  → 10xx xxxx
        //0xC0  1100 0000  → 110x xxxx
        PUSHC(c, (cp >> 6) | 0xC0);
        PUSHC(c, cp & 0x3F | 0x80);
        return 2;
    }
    else if (cp < 0x10000) {
        PUSHC(c, (cp >> 12) | 0xE0);
        PUSHC(c, (cp >> 6) & 0x3F | 0x80);
        PUSHC(c, cp & 0x3F | 0x80);
        return 3;
    }
    else if (cp <= 0x10FFFF) {
        PUSHC(c, (cp >> 18) | 0xF0);
        PUSHC(c, (cp >> 12) & 0x3F | 0x80);
        PUSHC(c, (cp >> 6) & 0x3F | 0x80);
        PUSHC(c, cp & 0x3F | 0x80);
        return 4;
    }
    else return 0;
}

/**
*   string = quotation-mark *char quotation-mark
*   char = unescaped /
*   escape (
*       %x22 /          ; "    quotation mark           U+0022
*       %x5C /          ; \    reverse solidus          U+005C
*       %x2F /          ; /    solidus                      U+002F
*       %x62 /          ; b    backspace                U+0008
*       %x66 /          ; f    form feed                U+000C
*       %x6E /          ; n    line feed                    U+000A
*       %x72 /          ; r    carriage return          U+000D
*       %x74 /          ; t    tab                          U+0009
*       %x75 4HEXDIG )  ; uXXXX                 U+XXXX
*   escape = %x5C          ; \
*   quotation-mark = %x22  ; "
*   unescaped = %x20-21 / %x23-5B / %x5D-10FFFF
**/
static int nfjson_parse_string_raw(nfjson_context *c, char **s, size_t *len) {
    *s = NULL;
    *len = 0;
    size_t begin = c->top;
    int ch, parse_status;
    unsigned int u;
    const char *str = c->json + 1;
    while (1) {
        switch (ch = *str++) {
        case '"':
            *len = c->top - begin;
            c->json = str;
            *s = nfjson_context_pop(c, *len);
            return NFJSON_PARSE_OK;
        case '\\':
            switch (ch = *str++) {
            case '"':PUSHC(c, '\"'); break;
            case '\\':PUSHC(c, '\\'); break;
            case '/':PUSHC(c, '/'); break;
            case 'b':PUSHC(c, '\b'); break;
            case 'f':PUSHC(c, '\f'); break;
            case 'n':PUSHC(c, '\n'); break;
            case 'r':PUSHC(c, '\r'); break;
            case 't':PUSHC(c, '\t'); break;
            case 'u':
                parse_status = nfjson_parse_unicode_char(&str, &u);
                if (parse_status == NFJSON_PARSE_OK) nfjson_encode_unicode_codepoint(c, u);
                else { c->top = begin; return parse_status; }
                break;
            default: c->top = begin; c->json = str; return NFJSON_PARSE_INVALID_STRING_ESCAPE;
            }break;
        case '\0':c->top = begin; return NFJSON_PARSE_MISS_QUOTATION_MARK;
        default:
            if (ch < 0x20 || ch > 0x10ffff) { c->top = begin; return NFJSON_PARSE_INVALID_STRING_CHAR; }
            PUSHC(c, ch);
            break;
        }
    }
}

static int nfjson_parse_string(nfjson_context *c, nfjson_value *val) {
    char *s;
    size_t len;
    int parse_status;
    if((parse_status = nfjson_parse_string_raw(c, &s, &len)) == NFJSON_PARSE_OK)
        nfjson_set_string(val, (const char *)s, len);
    return parse_status;
}

static int nfjson_parse_value();

/* array = %x5B ws [ value *( ws %x2C ws value ) ] ws %x5D */
static int nfjson_parse_array(nfjson_context *c, nfjson_value *val) {
    c->json++;
    nfjson_parse_whitespace(c);
    size_t len = 0;
    int parse_status = NFJSON_PARSE_OK;
    if (*c->json == ',') { parse_status = NFJSON_PARSE_EXPECT_VALUE; c->json++; }
    while (*c->json != ']') {
        nfjson_value *v = (nfjson_value *)malloc(sizeof(nfjson_value));
        nfjson_init(v);
        parse_status = nfjson_parse_value(c, v);
        if (parse_status == NFJSON_PARSE_OK) {
            *(uintptr_t *)nfjson_context_push(c, sizeof(uintptr_t)) = (uintptr_t)v;
            len++;
        }
        else { free(v); break; }
        nfjson_parse_whitespace(c);
        if (*c->json == ',') {
            c->json++;
            nfjson_parse_whitespace(c);
            if (*c->json == ']') { parse_status = NFJSON_EXTRA_COMMA; break; }
        }
        else if (*c->json != ']' || *c->json == '\0') {//value value
            parse_status = NFJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET; break;
        }
    }
    if (parse_status != NFJSON_PARSE_OK) {
        if (len) for (; len > 0; len--) {
            nfjson_value *nonuse = ((nfjson_value *)(*(uintptr_t *)nfjson_context_pop(c, sizeof(uintptr_t))));
            nfjson_free(nonuse);
            free(nonuse);
        }
    }else{
        c->json++;
        val->u.a.len = len;
        /*nfjson_value *array[] = malloc(sizeof(nfjson_value *)*len);*/ //expect continuous memory
        if (len) {
            nfjson_value *array = malloc(sizeof(nfjson_value)*len);
            memset(array, 0, sizeof(nfjson_value)*len);
            for (; len; len--) {
                //expect continuous memory
                /* *(array + len - 1) = (nfjson_value *)nfjson_context_pop(c, sizeof(uintptr_t)); */
                nfjson_value *vp = (nfjson_value *)(*(uintptr_t *)nfjson_context_pop(c, sizeof(uintptr_t)));
                *(array + len - 1) = *vp;
                free(vp);
            }
            val->u.a.e = array;
        }
        else val->u.a.e = NULL;
        val->type = JSON_ARRAY;
    }
    return parse_status;
}

static unsigned int nfjson_string_hashcode(nfjson_string *key) {//this func can not tell {"n\0", 1} & {"n\0", 2}, however, {"n\0", 1} may considered illegal
    unsigned int hash = 0;
    int i;
    char *s = key->s;
    for (i = (int)key->len; i >= 0; i--) {
        hash = hash * 33 + s[i];
    }
    return hash;
}

//look up key "abc\0abc" matches "abc\0", cmp thr len
static int cmp_nfjson_string_key(const nfjson_string *k, const nfjson_string *key) {
    return k->len == key->len && memcmp(k->s, key->s, k->len) == 0;
}

static void nfjson_value_free(nfjson_value *val) {
    nfjson_free(val);
    free(val);
}

static int nfjson_parse_nfjson_string(nfjson_context *c, nfjson_string *str) {
    str->s = NULL;
    str->len = 0;
    char *s;
    int parse_status;
    if ((parse_status = nfjson_parse_string_raw(c, &s, &str->len)) == NFJSON_PARSE_OK) {
        str->s = malloc(sizeof(char)*(str->len + 1));
        memcpy(str->s, s, str->len);
        str->s[str->len] = 0;
    }
    return parse_status;
}

/**
*   member = string ws %x3A ws value
*   object = %x7B ws [ member *( ws %x2C ws member ) ] ws %x7D
**/
static int nfjson_parse_object(nfjson_context *c, nfjson_value *val) {
    c->json++;
    nfjson_parse_whitespace(c);
    int parse_status = NFJSON_PARSE_OK;// {}
    char *s = NULL;
    nfjson_string *key;
    nfjson_value *value;
    void *old_val = NULL;
    hash_table *ht = new_hash_table(8, nfjson_string_hashcode, 
                                                            cmp_nfjson_string_key, nfjson_string_free, nfjson_value_free);
    while (*c->json != '}' && *c->json != '\0') {
        if (*c->json != '"') { parse_status = NFJSON_PARSE_MISS_KEY; break; }
        key = malloc(sizeof(nfjson_string));
        parse_status = nfjson_parse_nfjson_string(c, key);
        if (parse_status != NFJSON_PARSE_OK) { nfjson_string_free(key); parse_status = NFJSON_PARSE_MISS_KEY; break; }
        nfjson_parse_whitespace(c);
        if (*c->json++ != ':') {
            parse_status = NFJSON_PARSE_MISS_COLON; nfjson_string_free(key); break;
        }
        nfjson_parse_whitespace(c);
        value = malloc(sizeof(nfjson_value));
        nfjson_init(value);
        parse_status = nfjson_parse_value(c, value);
        if (parse_status != NFJSON_PARSE_OK) { nfjson_string_free(key); nfjson_free(value); free(value); break; }
        if (old_val = hash_table_put(ht, key, value)) { nfjson_free(old_val); free(old_val); }//repeated key
        nfjson_parse_whitespace(c);
        if (*c->json == '}') { break; }
        else if (*c->json == ',') {
            c->json++;
            nfjson_parse_whitespace(c);
            if (*c->json == '}' || *c->json == '\0') { parse_status = NFJSON_PARSE_MISS_KEY; break; }
        } else { parse_status = NFJSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET; break; }
    }
    if (*c->json == '}') c->json++;
    if (parse_status != NFJSON_PARSE_OK) {
        hash_table_free(ht);
    } else {
        val->u.ht = ht;
        val->type = JSON_OBJECT;
    }
    return parse_status;
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
    case '"':return nfjson_parse_string(c, val);
    case '[':return nfjson_parse_array(c, val);
    case '{':return nfjson_parse_object(c, val);
    case '\0': return NFJSON_PARSE_EXPECT_VALUE;
    default:  return NFJSON_PARSE_INVALID_VALUE;
    }
}

int nfjson_parse(nfjson_value *val, const char *json) {
    assert(NULL != val);
    nfjson_context context;
    context.json = json;
    context.stack = NULL;
    context.size = 0;
    context.top = 0;
    nfjson_init(val);
    nfjson_parse_whitespace(&context);
    int parse_status = nfjson_parse_value(&context, val);
    if (parse_status == NFJSON_PARSE_OK){
        nfjson_parse_whitespace(&context);
        if(*(context.json)) parse_status = NFJSON_PARSE_ROOT_NOT_SINGULAR;
    }
    assert(context.top == 0);
    free(context.stack);
    return parse_status;
}

#define PUSHS(c,str,len) do { memcpy(nfjson_context_push(c, len), str, len); }while(0)
static nfjson_stringify_string(nfjson_context *c, nfjson_string *str) {
    PUSHC(c, '"');
    //memcpy(nfjson_context_push(c, val->u.s.len), val->u.s.s, val->u.s.len);//'\0' -> "\u0000"
    char *s = str->s;
    size_t i = 0, len = str->len;
    for (; i < len; i++)
        if (s[i]) {
            switch (s[i]) {
            case '\"': PUSHS(c, "\\\"", 2); break;
            case '\\': PUSHS(c, "\\\\", 2); break;
            case '\b': PUSHS(c, "\\b", 2); break;
            case '\f': PUSHS(c, "\\f", 2); break;
            case '\n': PUSHS(c, "\\n", 2); break;
            case '\r': PUSHS(c, "\\r", 2); break;
            case '\t': PUSHS(c, "\\t", 2); break;
            //case '\/': PUSHC(c, '\"'); break;
            default:
                if (s[i] < 0x20) sprintf((char *)nfjson_context_push(c, 6), "\\u00%02X", s[i]);
                else PUSHC(c, s[i]);
            }
        }
        else memcpy(nfjson_context_push(c, 6), "\\u0000", 6);
    PUSHC(c, '"');
}

static int nfjson_stringify_value(nfjson_context *c, nfjson_value *val) {
    switch (val->type) {
    case JSON_NULL: PUSHS(c, "null", 4); break;
    case JSON_FALSE: PUSHS(c, "false", 5); break;
    case JSON_TRUE: PUSHS(c, "true", 4); break;
    case JSON_NUMBER: c->top -= 32 - sprintf((char *)nfjson_context_push(c, 32), "%.17g", val->u.n); break;//precison doubut
    case JSON_STRING: nfjson_stringify_string(c, &(val->u.s)); break;
    case JSON_ARRAY: 
    {
        PUSHC(c, '[');
        int len = (int)val->u.a.len - 1, i = 0;
        nfjson_value *array = val->u.a.e;
        for (; i < len; i++) {
            nfjson_stringify_value(c, array + i);
            PUSHC(c, ',');
        }
        if(array)nfjson_stringify_value(c, array + i);
        PUSHC(c, ']');
    }
    break;
    case JSON_OBJECT: 
    {
        PUSHC(c, '{');
        size_t size = (size_t)val->u.ht->cnt, i = 0, cnt = 0;
        kv **table = val->u.ht->table;
        kv* kv_list = NULL;
        while (cnt < size) {
            if (kv_list = table[i]) {
                while (kv_list) {
                    nfjson_stringify_string(c, (nfjson_string *)kv_list->key);
                    PUSHC(c, ':');
                    nfjson_stringify_value(c, (nfjson_value *)kv_list->val);
                    kv_list = kv_list->next;
                    PUSHC(c, ',');
                    cnt++;
                }
            }
            i++;
        }
        if(size) c->top--;//pop the last ','
        PUSHC(c, '}');
    }
    break;
    case JSON_UNRESOLVED: return NFJSON_STRINGIFY_UNRESOLVED_TYPE;
    default:return NFJSON_STRINGIFY_INVALID_TYPE;
    }
    return NFJSON_STRINGIFY_OK;
}

char *nfjson_stringify(nfjson_value *val, size_t *_len, int *_status) {
    nfjson_context *c = malloc(sizeof(nfjson_context));
    memset(c, 0, sizeof(nfjson_context));
    char *json = NULL;
    int status;
    if ((status = nfjson_stringify_value(c, val)) == NFJSON_STRINGIFY_OK) {
        size_t len = c->top;
        if (_len) *_len = len;
        json = malloc(sizeof(char)*(len + 1));
        memcpy(json, nfjson_context_pop(c, len), len);
        json[len] = 0;
    }
    if (_status) *_status = status;
    assert(c->top == 0);
    free(c->stack);
    free(c);
    return json;
}