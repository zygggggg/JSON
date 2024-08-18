#ifndef JSONPARSE_H__
#define JSONPARSE_H__

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

// 定义JSON值的结构体
typedef struct {
    lept_type type;  // JSON值的类型
}lept_value;

// 定义解析错误类型
enum {
    LEPT_PARSE_OK = 0,         // 解析成功
    LEPT_PARSE_EXPECT_VALUE,   // 期望得到值
    LEPT_PARSE_INVALID_VALUE,  // 无效的值
    LEPT_PARSE_ROOT_NOT_SINGULAR // 根节点不是单一的
};

// 解析JSON字符串
int lept_parse(lept_value* v, const char* json); 

// 获取解析后的JSON值的类型
lept_type lept_get_type(const lept_value* v);  

#endif /* JSONPARSE_H__ */