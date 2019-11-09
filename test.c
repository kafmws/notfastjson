#include"pch.h"
#include"notfastjson.h"
#include"parse.h"
#include"access.h"
#include"memory.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE(fabs((double)(expect) - (double)(actual))<1e6, (double)(expect), (double)(actual), "%f")
#define EXPECT_NOT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) != (actual), expect, actual, "%d")

#define TEST(expect, json, json_type) \
                do{\
                    nfjson_value v;\
                    EXPECT_EQ_INT(expect, nfjson_parse(&v, json));\
                    EXPECT_EQ_INT(json_type, nfjson_get_type(&v));\
                    nfjson_free(&v);\
                } while (0)

#define TEST_ERROR(expect, json) TEST(expect, json, JSON_UNRESOLVED)

#define TEST_OK(json_type, json) TEST(NFJSON_PARSE_OK, json, json_type)

#define TEST_NUMBER(expect, json) \
                do{\
                    nfjson_value v;\
                    EXPECT_EQ_INT(NFJSON_PARSE_OK, nfjson_parse(&v, json));\
                    EXPECT_EQ_INT(JSON_NUMBER, nfjson_get_type(&v));\
                    EXPECT_EQ_DOUBLE(expect, nfjson_get_number(&v));\
                    nfjson_free(&v);\
                }while(0)

static void test_parse_null() {
    TEST_OK(JSON_NULL, "null");
    TEST_OK(JSON_NULL, " null  ");
}

static void test_parse_true() {
    TEST_OK(JSON_TRUE, "true");
}

static void test_parse_false() {
    TEST_OK(JSON_FALSE, "false");
}

static void test_parse_invalid_value() {
    TEST_ERROR(NFJSON_PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(NFJSON_PARSE_INVALID_VALUE, "?");
    TEST_ERROR(NFJSON_PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(NFJSON_PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(NFJSON_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(NFJSON_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(NFJSON_PARSE_INVALID_VALUE, "0.");
    TEST_ERROR(NFJSON_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(NFJSON_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(NFJSON_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(NFJSON_PARSE_INVALID_VALUE, "nan");
}

static void test_parse_expect_value() {
    TEST_ERROR(NFJSON_PARSE_EXPECT_VALUE, "");
    TEST_ERROR(NFJSON_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_root_not_singular() {
    TEST(NFJSON_PARSE_ROOT_NOT_SINGULAR, "truee", JSON_TRUE);
    TEST(NFJSON_PARSE_ROOT_NOT_SINGULAR, "null x", JSON_NULL);
    TEST(NFJSON_PARSE_ROOT_NOT_SINGULAR, "123 .", JSON_NUMBER);
    TEST(NFJSON_PARSE_ROOT_NOT_SINGULAR, "0123", JSON_NUMBER); /* after zero should be '.' , 'E' , 'e' or nothing */
    TEST(NFJSON_PARSE_ROOT_NOT_SINGULAR, "0x0", JSON_NUMBER);
    TEST(NFJSON_PARSE_ROOT_NOT_SINGULAR, "0x123", JSON_NUMBER);

    TEST(NFJSON_PARSE_ROOT_NOT_SINGULAR, "00.1", JSON_NUMBER);
}

static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_NUMBER((double)123, "123 ");
    TEST_NUMBER(10E-308, "10E-308");
    TEST_NUMBER(10E307, "10e307");
    /* the smallest number > 1 */
    TEST_NUMBER(1.0000000000000002, "1.0000000000000002");
    /* minimum denormal */
    TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324");
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    /* Max subnormal double */
    TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308");
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    /* Min normal positive double */
    TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    /* Max double */
    TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void test_parse_number_too_big() {
    TEST_ERROR(NFJSON_PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(NFJSON_PARSE_NUMBER_TOO_BIG, "-1e309");
}

#define EXPECT_EQ_STRING(s1, s2, len) \
                EXPECT_EQ_BASE(sizeof(s1) - 1 == len && memcmp(s1, s2, len) == 0, s1, s2, "%s")

static void test_access_string() {
    nfjson_value val;
    nfjson_init(&val);
    nfjson_set_string(&val, "", 0);
    EXPECT_EQ_STRING("", nfjson_get_string(&val), nfjson_get_string_length(&val));
    nfjson_set_string(&val, "hello", 5);
    EXPECT_EQ_STRING("hello", nfjson_get_string(&val), nfjson_get_string_length(&val));
    nfjson_free(&val);
}

#define TEST_STRING(expect, json) \
               do{\
                    nfjson_value val;\
                    EXPECT_EQ_INT(NFJSON_PARSE_OK, nfjson_parse(&val, json)); \
                    EXPECT_EQ_INT(JSON_STRING, nfjson_get_type(&val)); \
                    EXPECT_EQ_STRING(expect, nfjson_get_string(&val), nfjson_get_string_length(&val)); \
                    nfjson_free(&val);\
               }while(0)

static void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
}

static void test_parse_missing_quotation_mark() {
    TEST_ERROR(NFJSON_PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(NFJSON_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
    TEST_ERROR(NFJSON_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(NFJSON_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(NFJSON_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(NFJSON_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
    TEST_ERROR(NFJSON_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(NFJSON_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_access_null() {
    nfjson_value v;
    nfjson_init(&v);
    nfjson_set_string(&v, "a", 1);
    nfjson_set_null(&v);
    EXPECT_EQ_INT(JSON_NULL, nfjson_get_type(&v));
    nfjson_free(&v);
}

#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")
static void test_access_boolean() {
    nfjson_value val;
    nfjson_init(&val);
    nfjson_set_string(&val, "1", 1);
    nfjson_set_boolean(&val, 1);
    EXPECT_TRUE(nfjson_get_boolean(&val));
    nfjson_set_boolean(&val, 0);
    EXPECT_FALSE(nfjson_get_boolean(&val));
    nfjson_free(&val);
}

static void test_access_number() {
    nfjson_value val;
    nfjson_init(&val);
    nfjson_set_number(&val, 15);
    EXPECT_EQ_DOUBLE((double)15, nfjson_get_number(&val));
    nfjson_set_number(&val, 0);
    EXPECT_EQ_DOUBLE((double)0, nfjson_get_number(&val));
    nfjson_set_number(&val, 1E5);
    EXPECT_EQ_DOUBLE(1e5, nfjson_get_number(&val));
    nfjson_free(&val);
}

static void test_parse_invalid_unicode_hex() {
    TEST_ERROR(NFJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_ERROR(NFJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_ERROR(NFJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_ERROR(NFJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_ERROR(NFJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_ERROR(NFJSON_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_ERROR(NFJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(NFJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_ERROR(NFJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u00/0\"");
    TEST_ERROR(NFJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_ERROR(NFJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_ERROR(NFJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
}

static void test_parse_invalid_unicode_surrogate() {
    TEST_ERROR(NFJSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_ERROR(NFJSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_ERROR(NFJSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_ERROR(NFJSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_ERROR(NFJSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((size_t)(expect)== (size_t)(actual), (size_t)(expect), (size_t)(actual), "%zu")

#define TEST_ARRAY_LITERAL(expect, array, index) \
            EXPECT_EQ_INT(expect, nfjson_get_type(nfjson_get_array_element(array, (index))))

#define TEST_ARRAY_NUMBER(expect, array, index) \
        do{\
            nfjson_value *__val = nfjson_get_array_element(array, (index));\
            EXPECT_EQ_INT(JSON_NUMBER, nfjson_get_type(__val));\
            EXPECT_EQ_DOUBLE((expect), nfjson_get_number(__val));\
        }while (0)

#define TEST_ARRAY_STRING(expect, array, index) \
        do{\
            nfjson_value *__val = nfjson_get_array_element(array, (index));\
            EXPECT_EQ_INT(JSON_STRING, nfjson_get_type(__val));\
            EXPECT_EQ_STRING(expect, nfjson_get_string(__val), strlen(expect));\
        }while (0)

#define TEST_ARRAY(val, json, array_size)\
        do{\
            EXPECT_EQ_INT(NFJSON_PARSE_OK, nfjson_parse(val, json));\
            EXPECT_EQ_INT(JSON_ARRAY, nfjson_get_type(val));\
            EXPECT_EQ_SIZE_T(array_size, nfjson_get_array_size(val));\
        }while (0)

static void test_parse_array() {
    nfjson_value v;
    TEST_ARRAY(&v, "[ ]", 0);
    nfjson_free(&v);
    TEST_ARRAY(&v, "[ null ,false ,  true,123,\"abc\" ]", 5);
    TEST_ARRAY_LITERAL(JSON_NULL, &v, 0);
    TEST_ARRAY_LITERAL(JSON_FALSE, &v, 1);
    TEST_ARRAY_LITERAL(JSON_TRUE, &v, 2);
    TEST_ARRAY_NUMBER(123, &v, 3);
    TEST_ARRAY_STRING("abc", &v, 4);
    nfjson_free(&v);
    TEST_ARRAY(&v, "[ [ ] , [0 ] ,[0,1],     [ 0 ,1  ,   2   ] ]", 4);
    nfjson_value *val = nfjson_get_array_element(&v, 0);
    EXPECT_EQ_SIZE_T(0, nfjson_get_array_size(val));
    val = nfjson_get_array_element(&v, 1);
    EXPECT_EQ_SIZE_T(1, nfjson_get_array_size(val));
    TEST_ARRAY_NUMBER(0, val, 0);
    val++;
    EXPECT_EQ_SIZE_T(2, nfjson_get_array_size(val));
    TEST_ARRAY_NUMBER(0, val, 0);
    TEST_ARRAY_NUMBER(1, val, 1);
    val++;
    EXPECT_EQ_SIZE_T(3, nfjson_get_array_size(val));
    TEST_ARRAY_NUMBER(0, val, 0);
    TEST_ARRAY_NUMBER(1, val, 1);
    TEST_ARRAY_NUMBER(2, val, 2);
    nfjson_free(&v);
    TEST_ARRAY(&v, "[ [ [],[[] ]] ]", 1);
    val = nfjson_get_array_element(&v, 0);
    EXPECT_EQ_SIZE_T(2, nfjson_get_array_size(val));
    val = nfjson_get_array_element(val, 0);
    EXPECT_EQ_SIZE_T(0, nfjson_get_array_size(val));
    val++;
    EXPECT_EQ_SIZE_T(1, nfjson_get_array_size(val));
    val = nfjson_get_array_element(val, 0);
    EXPECT_EQ_SIZE_T(0, nfjson_get_array_size(val));
    nfjson_free(&v);
    TEST_ARRAY(&v, "[ [ [],[[] ]] ]", 1);
    nfjson_free(&v);
}

static void test_parse() {
    #if 0
    test_parse_null();
    test_parse_true();
    test_parse_false();
    #endif
    test_parse_invalid_value();
    test_parse_expect_value();
    test_parse_root_not_singular();
    test_parse_number();
    test_parse_number_too_big();
    test_access_string();
    test_parse_string();
    test_parse_missing_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();
    test_access_null();
    test_access_boolean();
    test_access_number();
    test_parse_invalid_unicode_hex();
    test_parse_invalid_unicode_surrogate();
    test_parse_array();
}

int main() {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}