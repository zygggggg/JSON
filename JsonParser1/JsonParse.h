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

typedef struct {
    union {
        struct { char* s; size_t len; }s;  /* string: null-terminated string, string length */
        double n;                          /* number */
    }u;
    lept_type type;
}lept_value;



// 定义解析错误类型
enum {
    LEPT_PARSE_OK = 0,         // 解析成功
    LEPT_PARSE_EXPECT_VALUE,   // 期望得到值
    LEPT_PARSE_INVALID_VALUE,  // 无效的值
    LEPT_PARSE_ROOT_NOT_SINGULAR, // 根节点不是单一的
    LEPT_PARSE_NUMBER_TOO_BIG, // 数字太大
    LEPT_PARSE_MISS_QUOTATION_MARK, // 字符串没有引号
    LEPT_PARSE_INVALID_STRING_ESCAPE, // 字符串中的转义字符无效
    LEPT_PARSE_INVALID_STRING_CHAR // 字符串中的字符无效
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

#endif /* JSONPARSE_H__ */