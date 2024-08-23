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

// ����һ���꣬���ڼ�鵱ǰ�ַ��Ƿ���Ԥ���ַ���ƥ��
#define EXPECT(c, ch) do { \
    assert(*c->json == (ch)); \
    c->json++; \
} while(0)
// ��㷽���ж�0-9
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')
#define PUTC(c , ch)  do { *(char*)lept_context_push(c, sizeof(char)) = (ch); } while(0) // ���ص��ǿ�λ ��ch����ȥ
#define STRING_ERROR(ret) do { c->top = head; return ret; } while(0)

// ������������Ľṹ��
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
        if (c->json[i] != expect[i + 1]){return LEPT_PARSE_INVALID_VALUE;}  // �Ҵ������˺ܾã� ��c->json[i] ������ *c->json
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
    v->u.n = strtod(c->json, NULL);  // �����ȸ�Ϊ˫���� ���Ҵ���v->n
    if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    v->type = LEPT_NUMBER;
    c->json = p;
    return LEPT_PARSE_OK;
}

// �Ӹ������ַ��� p �н������ĸ�ʮ�������ַ���
// ��������ת����һ���޷������� u������ɹ��������򷵻�ָ���ַ�������һ���ַ���ָ�룻
// �������ʧ�ܣ���������ʮ�������ַ������򷵻� NULL��
static const char* lept_parse_hex4(const char* p, unsigned* u) {
    int i;
    *u = 0;
    // ѭ���ĴΣ�ÿ�δ���һ��ʮ�������ַ���
    for (i = 0; i < 4; i++) {
        // ��ȡ��ǰ�ַ���
        char ch = *p++;

        // �� u ������λ��Ϊ�¶�ȡ��ʮ�������ַ��ڳ��ռ䡣
        *u <<= 4;

        // �����ַ� ch ��ֵ������ת��Ϊ��Ӧ��ʮ��������ֵ��
        if (ch >= '0' && ch <= '9')  // ��� ch ��һ��ʮ�������֣�0-9����
            *u |= ch - '0';          // �� ch ת���ɶ�Ӧ����ֵ������ u ���а�λ�������
        else if (ch >= 'A' && ch <= 'F')  // ��� ch �Ǵ�д��ĸ A-F��
            *u |= ch - ('A' - 10);  // �� ch ת���ɶ�Ӧ����ֵ��10-15�������� u ���а�λ�������
        else if (ch >= 'a' && ch <= 'f')  // ��� ch ��Сд��ĸ a-f��
            *u |= ch - ('a' - 10);  // �� ch ת���ɶ�Ӧ����ֵ��10-15�������� u ���а�λ�������
        else  // ��� ch ������Ч��ʮ�������ַ���
            return NULL;  // ���� NULL ��ʾ����ʧ�ܡ�
    }

    // ����ɹ��������ĸ�ʮ�������ַ����򷵻�ָ���ַ�������һ���ַ���ָ�롣
    return p;
}

// ����2 ����strtol��׼�� ����ʮ����������

//static const char* lept_parse_hex4(const char* p, unsigned* u) {
//    char* end;
//    *u = (unsigned)strtol(p, &end, 16);
//    return end == p + 4 ? end : NULL;
//}


/*
U+20AC �� U+0800 ~ U+FFFF �ķ�Χ�ڣ�Ӧ����� 3 ���ֽڡ�
U+20AC �Ķ���λΪ 10000010101100
3 ���ֽڵ��������Ҫ 16 λ����㣬������ǰ�油���� 0����Ϊ 0010000010101100
���ϱ�Ѷ���λ�ֳ� 3 �飺0010, 000010, 101100
����ÿ���ֽڵ�ǰ׺��11100010, 10000010, 10101100
��ʮ����λ��ʾ����0xE2, 0x82, 0xAC
*/
/*
��� u�� 0x00 �� 0x7F ֮�䣬ʹ�� 1 �ֽڱ��롣
��� u�� 0x80 �� 0x7FF ֮�䣬ʹ�� 2 �ֽڱ��롣
���u�� 0x800 �� 0xFFFF ֮�䣬ʹ�� 3 �ֽڱ���.
��� u�� 0x10000 �� 0x10FFFF ֮�䣬ʹ�� 4 �ֽڱ��롣
*/
/*
��Ҫ�� u����һ���� ��ʹ����unsigned ͨ��������ʮ���� ����������Ϊʮ�������ڲ���
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

/*
��Ա�ļ�Ҳ��һ�� JSON �ַ�����Ȼ�������ǲ�ʹ�� lept_value �洢������Ϊ�������˷��˵��� type ������õ��ֶΡ�
���� lept_parse_string() ��ֱ�ӵذѽ����Ľ��д��һ�� lept_value��
�����������á���ȡ������extract method�������ع���ʽ���ѽ��� JSON �ַ�����д�� lept_value �ֲ�������֣�
*/
#define STRING_ERROR(ret) do { c->top = head; return ret; } while(0)

static int lept_parse_string_raw(lept_context* c, char** str, size_t* len) {
    size_t head;
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
                *len = c->top - head;
                *str = lept_context_pop(c, *len);
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
                    if (!(p = lept_parse_hex4(p, &u))) // ���ַ���תΪ��ֵ ����Ϊʮ������ �������ʧ�ܣ����� NULL��
                        STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
                    if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair */
                        if (*p++ != '\\') // ����һ��
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
                break; // ���break�ǹؼ���������

            default: // �������ͨ�ַ�
                if ((unsigned char)ch < 0x20)// ����ǿ����ַ�
                {
                    STRING_ERROR(LEPT_PARSE_INVALID_STRING_CHAR) ;
                }
                PUTC(c, ch); // ��ӽ�ȥ

        }
    }
}

static int lept_parse_string(lept_context* c, lept_value* v) {
    size_t len;
    char* s;
    int ret;
    if ((ret = lept_parse_string_raw(c, &s, &len)) == LEPT_PARSE_OK)
        lept_set_string(v, s, len);
    return ret;
}

//
//static int lept_parse_string(lept_context* c, lept_value* v)
//{
//    size_t head, len;
//    unsigned u, u2;
//    head = c->top;
//    char* p;
//    EXPECT(c, '\"');
//    p = c->json;
//    for (; ; )
//    {
//        char ch = *p++;
//        switch (ch)
//        {
//            case '\"':
//                len = c->top - head;
//                lept_set_string(v, (const char*)lept_context_pop(c, len), len);
//                c->json = p;
//                return LEPT_PARSE_OK;
//            case '\0':
//                c->top = head;
//                return LEPT_PARSE_MISS_QUOTATION_MARK;
//            case '\\':
//                switch (*p++)
//                {
//                    case '\"': PUTC(c, '\"'); break;
//                    case '\\': PUTC(c, '\\'); break;
//                    case '/':  PUTC(c, '/'); break;
//                    case 'b':  PUTC(c, '\b'); break;
//                    case 'f':  PUTC(c, '\f'); break;
//                    case 'n':  PUTC(c, '\n'); break;
//                    case 'r':  PUTC(c, '\r'); break;
//                    case 't':  PUTC(c, '\t'); break;
//                    case 'u': 
//                        if (!(p = lept_parse_hex4(p, &u))) // ���ַ���תΪ��ֵ ����Ϊʮ������ �������ʧ�ܣ����� NULL��
//                            STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
//                        if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair */
//                            if (*p++ != '\\') // ����һ��
//                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
//                            if (*p++ != 'u')
//                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
//                            if (!(p = lept_parse_hex4(p, &u2)))
//                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
//                            if (u2 < 0xDC00 || u2 > 0xDFFF)
//                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
//                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
//                        }
//                        lept_encode_utf8(c, u);
//                        break;
//                    default:
//                        STRING_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE);
//                }
//                break; // ���break�ǹؼ���������
//
//            default: // �������ͨ�ַ�
//                if ((unsigned char)ch < 0x20)// ����ǿ����ַ�
//                {
//                    c->top = head;
//                    return LEPT_PARSE_INVALID_STRING_CHAR;
//                }
//                PUTC(c, ch); // ��ӽ�ȥ
//
//        }
//    }
//}


static int lept_parse_value(lept_context* c, lept_value* v);

static int lept_parse_array(lept_context* c, lept_value* v) {
    size_t i, size = 0;
    int ret;
    EXPECT(c, '[');
    lept_parse_whitespace(c);
    // �����һ���ַ��� ']', ������Ϊ��
    if (*c->json == ']') {
        c->json++; // ���� ']' �ַ�
        v->type = LEPT_ARRAY;
        v->u.a.size = 0;
        v->u.a.e = NULL;
        return LEPT_PARSE_OK;
    }
    else {
        for (;;) {
            lept_value e;
            lept_init(&e); 
            if ((ret = lept_parse_value(c, &e)) != LEPT_PARSE_OK) {
                break; 
            }
            // �������õ���Ԫ��ѹ��������ջ
            memcpy(lept_context_push(c, sizeof(lept_value)), &e, sizeof(lept_value)); // e -> c
            size++; // Ԫ��������һ
            lept_parse_whitespace(c);
            // �����һ���ַ��� ',', �����������һ��Ԫ��
            if (*c->json == ',') {
                c->json++; 
                lept_parse_whitespace(c);
            }
            // �����һ���ַ��� ']', ���������
            else if (*c->json == ']') {
                c->json++; 
                v->type = LEPT_ARRAY;
                v->u.a.size = size;
                size *= sizeof(lept_value);
                memcpy(v->u.a.e = (lept_value*)malloc(size), lept_context_pop(c, size), size); // c -> v
                return LEPT_PARSE_OK;
            }
            else {
                ret = LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
                break;
            }
        }

        // ����ջ�ϵ�Ԫ��
        for (i = 0; i < size; i++) {
            lept_free((lept_value*)lept_context_pop(c, sizeof(lept_value)));
        }
            
        return ret;
    }
}

/*
{
  "name": "Alice",
  "age": 30,
  "isStudent": false,
  "hobbies": ["reading", "swimming", "traveling"],
  "address": {
    "street": "123 Main St",
    "city": "New York",
    "state": "NY"
  },
  "friends": [
    {"name": "Bob", "age": 28},
    {"name": "Charlie", "age": 32}
  ]
}
*/

static int lept_parse_object(lept_context* c, lept_value* v) {
    size_t i, size;
    lept_member m;
    int ret;
    EXPECT(c, '{');
    lept_parse_whitespace(c);
    if (*c->json == '}') {
        c->json++;
        v->type = LEPT_OBJECT;
        v->u.o.size = 0;
        v->u.o.m = NULL;
        return LEPT_PARSE_OK;
    }
    else {
        size = 0;
        m.k = NULL;
        for (; ;) {
            char* str;
            lept_init(&m.v); // ��ʼ����Աֵ
            if (*c->json != '"') {
                ret = LEPT_PARSE_MISS_KEY;
                break;
            }
            else {
                if (ret = lept_parse_string_raw(c, &str, &m.klen) != LEPT_PARSE_OK) { // �ع�string_raw ����Ϊ�˼�����ֱ����
                    break;
                }
                memcpy(m.k = (char*)malloc(m.klen + 1), str, m.klen); // ע�� �����m.kҪ����ռ䣡�� �����lept_context_push�Ѿ������˿ռ� ���õ�������
                m.k[m.klen] = '\0';
                lept_parse_whitespace(c);
            }
            if (*c->json != ':') {
                ret = LEPT_PARSE_MISS_COLON;
                break;
            }
            c->json++;
            lept_parse_whitespace(c);
            if ((ret = lept_parse_value(c, &m.v)) != LEPT_PARSE_OK) { // ������Աֵ
                break;
            }
            memcpy(lept_context_push(c, sizeof(lept_member)), &m, sizeof(lept_member));// m -> c ����Աѹ��������ջ
            size++;
            m.k = NULL;
            lept_parse_whitespace(c);
            if (*c->json == ',') {
                c->json++;
                lept_parse_whitespace(c);
            }
            else if (*c->json == '}') {
                size_t s = sizeof(lept_member) * size;
                c->json++;
                v->type = LEPT_OBJECT;
                v->u.o.size = size;
                memcpy(v->u.o.m = (lept_member*)malloc(s), lept_context_pop(c, s), s); // ���Ƴ�Ա���� c -> v
                return LEPT_PARSE_OK;
            }
            else {
                ret = LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
                break;
            }
            

        }
        free(m.k);
        for (i = 0; i < size; i++) {
            lept_member* m = (lept_member*)lept_context_pop(c, sizeof(lept_member));
            free(m->k);
            lept_free(&m->v);
        }
        v->type = LEPT_NULL;
        return ret;
    }
    
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
    case 't':  return lept_parse_new(c, v, "true", LEPT_TRUE);
    case 'f':  return lept_parse_new(c, v, "false", LEPT_FALSE);
    case 'n':  return lept_parse_new(c, v, "null", LEPT_NULL);
    default:   return lept_parse_number(c, v);
    case '"':  return lept_parse_string(c, v);
    case '[':  return lept_parse_array(c, v);
    case '{':  return lept_parse_object(c, v);
    case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) { 
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    c.stack = NULL;
    c.size = c.top = 0;
    lept_init(v);
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {// ע�⣡�� ����֮ǰ��� &c &v Ȼ��vԭ������ָ�룡 ���Դ��ˣ�
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
    size_t i;
    assert(v != NULL);
    switch (v->type) {
        case LEPT_STRING:
            free(v->u.s.s);
            break; // breakǧ��������
        case LEPT_ARRAY:
            for (i = 0; i < v->u.a.size; i++) {
                lept_free(&v->u.a.e[i]);// ���� & v->u.a.e[i]    ���e[i] !!! ��Ϊe��һ�����飨����ָ�룩 ����������Ƕ�� ����Ҫlept_free �����ǵ�����free
            }
            free(v->u.a.e);
            break;
        default: break;
    }
    v->type = LEPT_NULL;
}

// ��ȡ�������JSONֵ������

double lept_get_number(const lept_value* v) {
    assert(v != NULL);
    assert(v->type == LEPT_NUMBER);
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

size_t lept_get_array_size(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    return v->u.a.size;
}

lept_value* lept_get_array_element(const lept_value* v, size_t index) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    assert(index < v->u.a.size);
    return &v->u.a.e[index];
}

size_t lept_get_object_size(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_OBJECT);
    return v->u.o.size;
}

const char* lept_get_object_key(const lept_value* v, size_t index) {
    assert(v != NULL && v->type == LEPT_OBJECT);
    assert(index < v->u.o.size);
    return v->u.o.m[index].k;
}

size_t lept_get_object_key_length(const lept_value* v, size_t index) {
    assert(v != NULL && v->type == LEPT_OBJECT);
    assert(index < v->u.o.size);
    return v->u.o.m[index].klen;
}

lept_value* lept_get_object_value(const lept_value* v, size_t index) {
    assert(v != NULL && v->type == LEPT_OBJECT);
    assert(index < v->u.o.size);
    return &v->u.o.m[index].v;
}
