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
#define EXPECT_NOT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) != (actual), expect, actual, "%d")

static void test_parse_null() {
    nfjson_value v;
    EXPECT_EQ_INT(NFJSON_PARSE_OK, nfjson_parse(&v, "null"));
    EXPECT_EQ_INT(JSON_NULL, nfjson_get_type(&v));
    EXPECT_EQ_INT(NFJSON_PARSE_INVALID_VALUE, nfjson_parse(&v, "nul"));
    EXPECT_NOT_EQ_INT(JSON_NULL, nfjson_get_type(&v));
    EXPECT_EQ_INT(JSON_UNRESOLVED, nfjson_get_type(&v));
}

static void test_parse_true() {
    nfjson_value v;
    EXPECT_EQ_INT(NFJSON_PARSE_OK, nfjson_parse(&v, "true"));
    EXPECT_EQ_INT(JSON_TRUE, nfjson_get_type(&v));
    EXPECT_EQ_INT(NFJSON_PARSE_ROOT_NOT_SINGULAR, nfjson_parse(&v, "truee"));
    EXPECT_EQ_INT(JSON_TRUE, nfjson_get_type(&v));
}

static void test_parse_false() {
    nfjson_value v;
    EXPECT_EQ_INT(NFJSON_PARSE_EXPECT_VALUE, nfjson_parse(&v, ""));
    EXPECT_EQ_INT(JSON_UNRESOLVED, nfjson_get_type(&v));
    EXPECT_EQ_INT(NFJSON_PARSE_OK, nfjson_parse(&v, "false"));
    EXPECT_EQ_INT(JSON_FALSE, nfjson_get_type(&v));
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
}

int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}