#ifndef JSONPARSE_H__
#define JSONPARSE_H__

#include <stddef.h> /* size_t */

// 定义JSON值的类型
typedef enum { 
    LEPT_NULL,      // Null类型
    LEPT_FALSE,     // Boolean值false
    LEPT_TRUE,      // Boolean值true
    LEPT_NUMBER,    // 数字类型
    LEPT_STRING,    // 字符串类型
    LEPT_ARRAY,     // 数组类型
    LEPT_OBJECT     // 对象类型
} lept_type;

//一定要注意数据结构的设计！
typedef struct lept_value lept_value;
typedef struct lept_member lept_member;

struct lept_value {
    union {
        struct { lept_member* m; size_t size; }o; // 对象
        struct { lept_value* e; size_t size; }a;    /* array:  elements, element count */
        struct { char* s; size_t len; }s;           /* string: null-terminated string, string length */
        double n;                                   /* number */
    }u;
    lept_type type;
};



struct lept_member {
    char* k;          // 键字符串
    size_t klen;      // 键字符串的长度
    lept_value v;     // 成员值
};

/**
 * @brief JSON 解析错误码枚举。
 *
 * 定义了 JSON 解析过程中可能出现的各种错误码。
 */
enum {
    LEPT_PARSE_OK = 0, // 解析成功
    LEPT_PARSE_EXPECT_VALUE, // 期望值但未找到
    LEPT_PARSE_INVALID_VALUE, // 无效的值
    LEPT_PARSE_ROOT_NOT_SINGULAR, // 根节点不是单一值
    LEPT_PARSE_NUMBER_TOO_BIG, // 数字太大无法解析
    LEPT_PARSE_MISS_QUOTATION_MARK, // 缺少引号
    LEPT_PARSE_INVALID_STRING_ESCAPE, // 无效的字符串转义字符
    LEPT_PARSE_INVALID_STRING_CHAR, // 无效的字符串字符
    LEPT_PARSE_INVALID_UNICODE_HEX, // 无效的 Unicode 十六进制值
    LEPT_PARSE_INVALID_UNICODE_SURROGATE, // 无效的 Unicode 替代字符
    LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, // 缺少逗号或方括号
    LEPT_PARSE_MISS_KEY, // 缺少键
    LEPT_PARSE_MISS_COLON, // 缺少冒号
    LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET // 缺少逗号或大括号
};


#define lept_init(v) do { (v)->type = LEPT_NULL; } while(0)


// 解析JSON字符串
int lept_parse(lept_value* v, const char* json); 

void lept_free(lept_value* v);
// 获取解析后的JSON值的类型
lept_type lept_get_type(const lept_value* v);

#define lept_set_null(v) lept_free(v)


double lept_get_number(const lept_value* v);
void set_number(lept_value* v, double n);

int lept_get_boolean(const lept_value* v);
void set_boolean(lept_value* v, int b);

const char* lept_get_string(const lept_value* v);   //v指向的对象不会被修改 字符串是只读的
size_t lept_get_string_length(const lept_value* v);
void lept_set_string(lept_value* v, const char* s, size_t len); // 学习char* 替代string 

size_t lept_get_array_size(const lept_value* v);
lept_value* lept_get_array_element(const lept_value* v, size_t index);

size_t lept_get_object_size(const lept_value* v); // 获取对象成员个数
const char* lept_get_object_key(const lept_value* v, size_t index);
size_t lept_get_object_key_length(const lept_value* v, size_t index);
lept_value* lept_get_object_value(const lept_value* v, size_t index);


#endif /* JSONPARSE_H__ */