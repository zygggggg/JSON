#include "JsonParse.h"
#include <assert.h> 
#include <stdlib.h>  
#include <errno.h>
#include <math.h>

// 定义一个宏，用于检查当前字符是否与预期字符相匹配
#define EXPECT(c, ch) do { \
    assert(*c->json == (ch)); \
    c->json++; \
} while(0)
// 简便方法判断0-9
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')


// 定义解析上下文结构体
typedef struct {
    const char* json;  // 指向待解析的JSON字符串
}lept_context;

void lept_parse_whitespace(lept_context *c)
{
    const char* p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_new (lept_context *c , lept_value *v , const char* expect , lept_type type)
{
    size_t i;
    EXPECT(c , expect[0]);
    for (i = 0 ; expect[i + 1] ; i++)
    {
        if (c->json[i] != expect[i + 1]){return LEPT_PARSE_INVALID_VALUE;}  // 找错误找了很久！ 是c->json[i] 而不是 *c->json
    }
    c->json += i;
    v->type = type;
    return LEPT_PARSE_OK;
}

/*  重复代码 重构！
static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3; // 移动指针到true之后
    v->type = LEPT_TRUE; // 设置解析结果的类型
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}
*/

static int lept_parse_number(lept_context* c, lept_value* v) {
    const char* p = c->json;
    if (*p == '-') p++;
    if (*p == '0') p++;
    else {
        if (!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == '.') {
        p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    errno = 0;
    v->n = strtod(c->json, NULL);  // 单精度改为双精度 并且存在v->n
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    v->type = LEPT_NUMBER;
    c->json = p;
    return LEPT_PARSE_OK;
}


static int lept_parse_value(lept_context *c , lept_value *v)
{
    switch (*c->json)
    {
        case 't':  return lept_parse_new(c, v, "true", LEPT_TRUE);
        case 'f':  return lept_parse_new(c, v, "false", LEPT_FALSE);
        case 'n':  return lept_parse_new(c, v, "null", LEPT_NULL);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json)
{
    lept_context c;
    c.json = json;
    v->type = LEPT_NULL;
    assert(v != NULL); //assert(v != NULL); 是一种防御性编程的做法，用于确保传入的指针不为空
    lept_parse_whitespace(&c);
    int ret;
    if ((ret = lept_parse_value(&c , v)) == LEPT_PARSE_OK) // 注意！！ 这里之前搞成 &c &v 然而v原本就是指针！ 所以错了！
    {
        if(*c.json != '\0')
        {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(lept_value *v)
{
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
