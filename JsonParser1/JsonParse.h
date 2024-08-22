#ifndef JSONPARSE_H__
#define JSONPARSE_H__

#include <stddef.h> /* size_t */

// ����JSONֵ������
typedef enum { 
    LEPT_NULL,      // Null����
    LEPT_FALSE,     // Booleanֵfalse
    LEPT_TRUE,      // Booleanֵtrue
    LEPT_NUMBER,    // ��������
    LEPT_STRING,    // �ַ�������
    LEPT_ARRAY,     // ��������
    LEPT_OBJECT     // ��������
} lept_type;

typedef struct {
    union {
        struct { char* s; size_t len; }s;  /* string: null-terminated string, string length */
        double n;                          /* number */
    }u;
    lept_type type;
}lept_value;



// ���������������
enum {
    LEPT_PARSE_OK = 0,         // �����ɹ�
    LEPT_PARSE_EXPECT_VALUE,   // �����õ�ֵ
    LEPT_PARSE_INVALID_VALUE,  // ��Ч��ֵ
    LEPT_PARSE_ROOT_NOT_SINGULAR, // ���ڵ㲻�ǵ�һ��
    LEPT_PARSE_NUMBER_TOO_BIG, // ����̫��
    LEPT_PARSE_MISS_QUOTATION_MARK, // �ַ���û������
    LEPT_PARSE_INVALID_STRING_ESCAPE, // �ַ����е�ת���ַ���Ч
    LEPT_PARSE_INVALID_STRING_CHAR // �ַ����е��ַ���Ч
};


#define lept_init(v) do { (v)->type = LEPT_NULL; } while(0)


// ����JSON�ַ���
int lept_parse(lept_value* v, const char* json); 

void lept_free(lept_value* v);
// ��ȡ�������JSONֵ������
lept_type lept_get_type(const lept_value* v);

#define lept_set_null(v) lept_free(v)


double lept_get_number(const lept_value* v);
void set_number(lept_value* v, double n);

int lept_get_boolean(const lept_value* v);
void set_boolean(lept_value* v, int b);

const char* lept_get_string(const lept_value* v);   //vָ��Ķ��󲻻ᱻ�޸� �ַ�����ֻ����
size_t lept_get_string_length(const lept_value* v);
void lept_set_string(lept_value* v, const char* s, size_t len); // ѧϰchar* ���string 

#endif /* JSONPARSE_H__ */