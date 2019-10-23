#include"pch.h"
#include"notfastjson.h"

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
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE(fabs(expect - actual)<1e6, expect, actual, "%lf")
#define EXPECT_NOT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) != (actual), expect, actual, "%d")

#define TEST(expect, json, json_type) \
                do{\
                    nfjson_value v;\
                    EXPECT_EQ_INT(expect, nfjson_parse(&v, json));\
                    EXPECT_EQ_INT(json_type, nfjson_get_type(&v));\
                } while (0)

#define TEST_ERROR(expect, json) TEST(expect, json, JSON_UNRESOLVED)

#define TEST_OK(json_type, json) TEST(NFJSON_PARSE_OK, json, json_type)

#define TEST_NUMBER(expect, json) \
                do{\
                nfjson_value v;\
                EXPECT_EQ_INT(NFJSON_PARSE_OK, nfjson_parse(&v, json));\
                EXPECT_EQ_INT(JSON_NUMBER, nfjson_get_type(&v));\
                EXPECT_EQ_DOUBLE(expect, nfjson_get_number(&v));\
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
}

int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    nfjson_value v;
    return main_ret;
}