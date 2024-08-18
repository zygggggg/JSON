#include "JsonParse.h"
#include <assert.h> 
#include <stdlib.h>  

// ����һ���꣬���ڼ�鵱ǰ�ַ��Ƿ���Ԥ���ַ���ƥ��
#define EXPECT(c, ch) do { \
    assert(*c->json == (ch)); \
    c->json++; \
} while(0)

// ������������Ľṹ��
typedef struct {
    const char* json;  // ָ���������JSON�ַ���
}lept_context;

void lept_parse_whitespace(lept_context *c)
{
    const char* p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3; // �ƶ�ָ�뵽true֮��
    v->type = LEPT_TRUE; // ���ý������������
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

int lept_parse_value(lept_context *c , lept_value *v)
{
    switch (*c->json)
    {
        case 't': return lept_parse_true(c , v);
        case 'f': return lept_parse_false(c , v);
        case 'n': return lept_parse_null(c , v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
        default:   return LEPT_PARSE_INVALID_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json)
{
    lept_context c;
    c.json = json;
    v->type = LEPT_NULL;
    assert(v != NULL); //assert(v != NULL); ��һ�ַ����Ա�̵�����������ȷ�������ָ�벻Ϊ��
    lept_parse_whitespace(&c);
    int ret;
    if ((ret = lept_parse_value(&c , &v)) == LEPT_PARSE_OK)
    {
        if(*c.json != '\0')
        {
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