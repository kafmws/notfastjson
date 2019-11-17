# notfastjson
&copy; 2019 kafm ([wdxdesperado@qq.com](http://wdxdesperado@qq.com))
### 简介
简单 不是很快的 JSON解析库&emsp;C语言实现
- ANSI C标准，跨平台 / 编译器
- 递归下降解析器
- 支持UTF-8字符
- number类型数字解析仅支持C语言double类型范围
- 参考 [从零开始的 JSON 库教程](https://zhuanlan.zhihu.com/json-tutorial) @ [Milo Yip](https://github.com/miloyip)

### 构建
原项目Win平台下使用VS编写，使用```<crtdbg.h>```及```_CRTDBG_MAP_ALLOC```宏检测内存泄漏
使用宏
```c
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
```
构建简单单元测试

### JSON类型
|类型|enum 表示|
|:--:|:--:|
|null|JSON_NULL|
|bool|JSON_TRUE / JSON_FALSE|
|number|JSON_NUMBER|
|string|JSON_STRING|
|object|JSON_OBJECT|

### 数据结构
```c
typedef struct nfjson_value nfjson_value;

typedef struct { char *s; size_t len; } nfjson_string;

struct nfjson_value {
    union {
        nfjson_string s;
        struct { nfjson_value *e; size_t len; }a;/* type == JSON_ARRAY */
        hash_table *hto;/* type == JSON_OBJECT */
        double n;/* type == JSON_NUMBER */
    }u;
    nfjson_type type;
};

typedef struct {
    const char *json;/*the parsing position in the json*/
    char *stack;/*parsing buffer*/
    size_t size;/*size of stack*/
    size_t top;/*pointer of stack*/
}nfjson_context;
```

|类型|意义/实现方式|
|:--:|:--:|
|nfjson_context|JSON字符串解析信息|
|nfjson_value|解析出的JSON值|
|nfjson_string|UTF-8字符串值|
|hash_table|通用hash表，<br>内置使用以nfjson_string为key类型，<br>nfjson_value为value类型使用|
|nfjson_context 中 stack|任意类型动态堆栈，<br>用作string，array类型解析缓冲区|

### 文件结构
```
|-- access.c .h				//getter & setter
|-- hash_table.c .h			//hash_table
|-- memory.c .h				//manage memory
|-- nfjson.h				//define data structure, error code & json type
|-- parse.c .h				//recursive-descent json parser
|-- pch.c .h				//VS required
|-- test.c .h				//unit testing
```

### 其它
- **内存管理** ```malloc```，```realloc```，```free```，```nfjson_string_free```及递归```nfjson_free```
- **缓冲堆栈** ```realloc```扩容，任意类型，悬浮指针问题
- **哈希支持** 任意类型，按需配置

### 示例
```c
nfjson_value val;

nfjson_parse(&val, 
				" { "
				"\"n\" : null , "
				"\"f\" : false , "
				"\"t\" : true , "
				"\"i\" : 123 , "
				"\"s\" : \"abc\", "
				"\"a\" : [ 1, 2, 3 ],"
				"\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
				"}"
);
nfjson_get_type(&val) == JSON_OBJECT;
nfjson_get_object_size(&val) == 7;
nfjson_string []s = 
			{ {"n", 1},{"f", 1},{"t", 1},{"i", 1},{"s", 1},{"a", 1},{"o", 1}, };

//access { "n" : null }
nfjson_value *v = nfjson_get_object_value(&s[0]);
nfjson_get_type(&v) == JSON_NULL;
//access { "i" : 123 }
v = nfjson_get_object_value(&s[3]);
nfjson_get_number(&v) == 123;
//access 1 in { "a" : [1, 2, 3] }
v = nfjson_get_object_value(&s[5]);
nfjson_get_type(&val) == JSON_ARRAY;
nfjson_get_array_size(&val) == 3;
v = nfjson_get_array_element(&v, 1);
nfjson_get_type(&val) == JSON_NUMBER;
nfjson_get_number(&v) == 1;
```
更多示例见 ```test.c```

---

# notfastjson
&copy; 2019 kafm ([wdxdesperado@qq.com](http://wdxdesperado@qq.com))
### Introduction
 a simple and not fast json parsing library writed in C
- standard ANSI C，cross-platform / compiler
- recursive decent parser
- UTF-8 char support
- number in range of ```double```in C only
- based on [json-tutorial](https://zhuanlan.zhihu.com/json-tutorial) @ [Milo Yip](https://github.com/miloyip)
### Build
write in Windows with Visual Studio，using ```<crtdbg.h>```& macro ```_CRTDBG_MAP_ALLOC```to detect memory leaks
using following marco
```c
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
```
build simple unit testing.

### JSON type
|type|enum|
|:--:|:--:|
|null|JSON_NULL|
|bool|JSON_TRUE / JSON_FALSE|
|number|JSON_NUMBER|
|string|JSON_STRING|
|object|JSON_OBJECT|

### Data Structure
```c
typedef struct nfjson_value nfjson_value;

typedef struct { char *s; size_t len; } nfjson_string;

struct nfjson_value {
    union {
        nfjson_string s;
        struct { nfjson_value *e; size_t len; }a;/* type == JSON_ARRAY */
        hash_table *hto;/* type == JSON_OBJECT */
        double n;/* type == JSON_NUMBER */
    }u;
    nfjson_type type;
};

typedef struct {
    const char *json;/*the parsing position in the json*/
    char *stack;/*parsing buffer*/
    size_t size;/*size of stack*/
    size_t top;/*pointer of stack*/
}nfjson_context;
```

|type define|meaning/implement|
|:--:|:--:|
|nfjson_context|JSON string context in parsing|
|nfjson_value|json value|
|nfjson_string|UTF-8 string|
|hash_table|a hash table,<br>using nfjson_string as key<br>and nfjson_value as value|
|stack in nfjson_context|dynamic stack for any type,<br>as a buffer for json string type<br>and json array type in parsing|

### file structure
```
|-- access.c .h				//getter & setter
|-- hash_table.c .h			//hash_table
|-- memory.c .h				//manage memory
|-- nfjson.h				//define data structure, error code & json type
|-- parse.c .h				//recursive-descent json parser
|-- pch.c .h				//VS required
|-- test.c .h				//unit testing
```

### Other
- **memory managment** ```malloc```，```realloc```，```free```，```nfjson_string_free```& recursive ```nfjson_free```
- **buffer stack** ```realloc```dynamic expansion，support any type，suspensive pointer problem
- **hash table support** support any type，on-demand configure

### examples
```c
nfjson_value val;

nfjson_parse(&val, 
				" { "
				"\"n\" : null , "
				"\"f\" : false , "
				"\"t\" : true , "
				"\"i\" : 123 , "
				"\"s\" : \"abc\", "
				"\"a\" : [ 1, 2, 3 ],"
				"\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
				"}"
);
nfjson_get_type(&val) == JSON_OBJECT;
nfjson_get_object_size(&val) == 7;
nfjson_string []s = 
			{ {"n", 1},{"f", 1},{"t", 1},{"i", 1},{"s", 1},{"a", 1},{"o", 1}, };

//access { "n" : null }
nfjson_value *v = nfjson_get_object_value(&s[0]);
nfjson_get_type(&v) == JSON_NULL;
//access { "i" : 123 }
v = nfjson_get_object_value(&s[3]);
nfjson_get_number(&v) == 123;
//access 1 in { "a" : [1, 2, 3] }
v = nfjson_get_object_value(&s[5]);
nfjson_get_type(&val) == JSON_ARRAY;
nfjson_get_array_size(&val) == 3;
v = nfjson_get_array_element(&v, 1);
nfjson_get_type(&val) == JSON_NUMBER;
nfjson_get_number(&v) == 1;
```
for more examples, see  ```test.c```
