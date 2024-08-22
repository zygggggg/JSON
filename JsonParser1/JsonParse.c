#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "JsonParse.h"
#include <assert.h>  /* assert() */
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <stdlib.h>  /* NULL, malloc(), realloc(), free(), strtod() */
#include <string.h>  /* memcpy() */

#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

// 定义一个宏，用于检查当前字符是否与预期字符相匹配
#define EXPECT(c, ch) do { \
    assert(*c->json == (ch)); \
    c->json++; \
} while(0)
// 简便方法判断0-9
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')
#define PUTC(c , ch)  do { *(char*)lept_context_push(c, sizeof(char)) = (ch); } while(0) // 返回的是空位 让ch补进去
#define STRING_ERROR(ret) do { c->top = head; return ret; } while(0)

// 定义解析上下文结构体
typedef struct {
    const char* json;
    char* stack;
    size_t size, top;
}lept_context;


static void* lept_context_push(lept_context* c, size_t size) {
    void* ret;
    assert(size > 0);
    if (c->top + size >= c->size) {
        if (c->size == 0)
            c->size = LEPT_PARSE_STACK_INIT_SIZE;
        while (c->top + size >= c->size)
            c->size += c->size >> 1;  /* c->size * 1.5 */
        c->stack = (char*)realloc(c->stack, c->size);
    }
    ret = c->stack + c->top;
    c->top += size;
    return ret;
}


static void* lept_context_pop(lept_context* c, size_t size) {
    assert(c->top >= size);
    c->top -= size;
    return c->stack + c->top;
}

void lept_parse_whitespace(lept_context *c)
{
    const char* p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_new(lept_context *c , lept_value *v , const char* expect , lept_type type)
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
    v->u.n = strtod(c->json, NULL);  // 单精度改为双精度 并且存在v->n
    if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    v->type = LEPT_NUMBER;
    c->json = p;
    return LEPT_PARSE_OK;
}

// 从给定的字符串 p 中解析出四个十六进制字符，
// 并将它们转换成一个无符号整数 u。如果成功解析，则返回指向字符串中下一个字符的指针；
// 如果解析失败（即遇到非十六进制字符），则返回 NULL。
static const char* lept_parse_hex4(const char* p, unsigned* u) {
    int i;
    *u = 0;
    // 循环四次，每次处理一个十六进制字符。
    for (i = 0; i < 4; i++) {
        // 获取当前字符。
        char ch = *p++;

        // 将 u 左移四位，为新读取的十六进制字符腾出空间。
        *u <<= 4;

        // 根据字符 ch 的值，将其转换为对应的十六进制数值。
        if (ch >= '0' && ch <= '9')  // 如果 ch 是一个十进制数字（0-9）。
            *u |= ch - '0';          // 将 ch 转换成对应的数值，并与 u 进行按位或操作。
        else if (ch >= 'A' && ch <= 'F')  // 如果 ch 是大写字母 A-F。
            *u |= ch - ('A' - 10);  // 将 ch 转换成对应的数值（10-15），并与 u 进行按位或操作。
        else if (ch >= 'a' && ch <= 'f')  // 如果 ch 是小写字母 a-f。
            *u |= ch - ('a' - 10);  // 将 ch 转换成对应的数值（10-15），并与 u 进行按位或操作。
        else  // 如果 ch 不是有效的十六进制字符。
            return NULL;  // 返回 NULL 表示解析失败。
    }

    // 如果成功解析了四个十六进制字符，则返回指向字符串中下一个字符的指针。
    return p;
}

/*
U+20AC 在 U+0800 ~ U+FFFF 的范围内，应编码成 3 个字节。
U+20AC 的二进位为 10000010101100
3 个字节的情况我们要 16 位的码点，所以在前面补两个 0，成为 0010000010101100
按上表把二进位分成 3 组：0010, 000010, 101100
加上每个字节的前缀：11100010, 10000010, 10101100
用十六进位表示即：0xE2, 0x82, 0xAC
*/
/*
如果 u在 0x00 到 0x7F 之间，使用 1 字节编码。
如果 u在 0x80 到 0x7FF 之间，使用 2 字节编码。
如果u在 0x800 到 0xFFFF 之间，使用 3 字节编码.
如果 u在 0x10000 到 0x10FFFF 之间，使用 4 字节编码。
*/
/*
重要！ u就是一个数 即使他是unsigned 通常觉得是十进制 这里想象他为十六进制在操作
*/
static void lept_encode_utf8(lept_context* c, unsigned u) {
    if (u <= 0x7F)
        PUTC(c, u & 0xFF);
    else if (u <= 0x7FF) {
        PUTC(c, 0xC0 | ((u >> 6) & 0xFF));
        PUTC(c, 0x80 | (u & 0x3F));
    }
    else if (u <= 0xFFFF) {
        PUTC(c, 0xE0 | ((u >> 12) & 0xFF));
        PUTC(c, 0x80 | ((u >> 6) & 0x3F));
        PUTC(c, 0x80 | (u & 0x3F));
    }
    else {
        assert(u <= 0x10FFFF);
        PUTC(c, 0xF0 | ((u >> 18) & 0xFF));
        PUTC(c, 0x80 | ((u >> 12) & 0x3F));
        PUTC(c, 0x80 | ((u >> 6) & 0x3F));
        PUTC(c, 0x80 | (u & 0x3F));
    }
}


static int lept_parse_string(lept_context* c, lept_value* v)
{
    size_t head, len;
    unsigned u, u2;
    head = c->top;
    char* p;
    EXPECT(c, '\"');
    p = c->json;
    for (; ; )
    {
        char ch = *p++;
        switch (ch)
        {
            case '\"':
                len = c->top - head;
                lept_set_string(v, (const char*)lept_context_pop(c, len), len);
                c->json = p;
                return LEPT_PARSE_OK;
            case '\0':
                c->top = head;
                return LEPT_PARSE_MISS_QUOTATION_MARK;
            case '\\':
                switch (*p++)
                {
                    case '\"': PUTC(c, '\"'); break;
                    case '\\': PUTC(c, '\\'); break;
                    case '/':  PUTC(c, '/'); break;
                    case 'b':  PUTC(c, '\b'); break;
                    case 'f':  PUTC(c, '\f'); break;
                    case 'n':  PUTC(c, '\n'); break;
                    case 'r':  PUTC(c, '\r'); break;
                    case 't':  PUTC(c, '\t'); break;
                    case 'u': 
                        if (!(p = lept_parse_hex4(p, &u))) // 由字符串转为数值 想象为十六进制 如果解析失败，返回 NULL。
                            STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
                        if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair */
                            if (*p++ != '\\') // 再来一次
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            if (*p++ != 'u')
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            if (!(p = lept_parse_hex4(p, &u2)))
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
                            if (u2 < 0xDC00 || u2 > 0xDFFF)
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        lept_encode_utf8(c, u);
                        break;
                    default:
                        STRING_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE);
                }
                break; // 这个break是关键！！！！

            default: // 如果是普通字符
                if ((unsigned char)ch < 0x20)// 如果是控制字符
                {
                    c->top = head;
                    return LEPT_PARSE_INVALID_STRING_CHAR;
                }
                PUTC(c, ch); // 添加进去

        }
    }
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
        case '"': return lept_parse_string(c, v);
    }
}

//int lept_parse(lept_value* v, const char* json)
//{
//    lept_context c;
//    c.json = json;
//    v->type = LEPT_NULL;
//    assert(v != NULL); //assert(v != NULL); 是一种防御性编程的做法，用于确保传入的指针不为空
//    lept_parse_whitespace(&c);
//    int ret;
//    if ((ret = lept_parse_value(&c , v)) == LEPT_PARSE_OK) // 注意！！ 这里之前搞成 &c &v 然而v原本就是指针！ 所以错了！
//    {
//        if(*c.json != '\0')
//        {
//            v->type = LEPT_NULL;
//            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
//        }
//    }
//    return ret;
//}
int lept_parse(lept_value* v, const char* json) { 
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    c.stack = NULL;
    c.size = c.top = 0;
    lept_init(v);
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {// 注意！！ 这里之前搞成 &c &v 然而v原本就是指针！ 所以错了！
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    assert(c.top == 0);
    free(c.stack);
    return ret;
}



lept_type lept_get_type(lept_value *v)
{
    assert(v != NULL);
    return v->type;
}


void lept_free(lept_value* v) {
    assert(v != NULL);
    if (v->type == LEPT_STRING) {
        free(v->u.s.s);
    }
    v->type = LEPT_NULL;
}
// 获取解析后的JSON值的类型

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->u.n;
}
void lept_set_number(lept_value* v, double n) {
    lept_free(v);
    v->u.n = n;
    v->type = LEPT_NUMBER;
}

int lept_get_boolean(const lept_value* v) {
    assert(v != NULL && (v->type == LEPT_TRUE || v->type == LEPT_FALSE));
    return v->type == LEPT_TRUE;
}
void lept_set_boolean(lept_value* v, int b) {
    lept_free(v);
    v->type = b ? LEPT_TRUE : LEPT_FALSE;
}

const char* lept_get_string(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->u.s.s;
}

size_t lept_get_string_length(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->u.s.len;
}

void lept_set_string(lept_value* v, const char* s, size_t len) {
    lept_free(v);
    assert(s != NULL || len == 0);
    assert(v != NULL);
    v->u.s.s = (char*)malloc(len + 1);
    memcpy(v->u.s.s, s, len);
    v->u.s.s[len] = '\0';
    v->type = LEPT_STRING;
    v->u.s.len = len;
}