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

//һ��Ҫע�����ݽṹ����ƣ�
typedef struct lept_value lept_value;
typedef struct lept_member lept_member;

struct lept_value {
    union {
        struct { lept_member* m; size_t size; }o; // ����
        struct { lept_value* e; size_t size; }a;    /* array:  elements, element count */
        struct { char* s; size_t len; }s;           /* string: null-terminated string, string length */
        double n;                                   /* number */
    }u;
    lept_type type;
};



struct lept_member {
    char* k;          // ���ַ���
    size_t klen;      // ���ַ����ĳ���
    lept_value v;     // ��Աֵ
};

/**
 * @brief JSON ����������ö�١�
 *
 * ������ JSON ���������п��ܳ��ֵĸ��ִ����롣
 */
enum {
    LEPT_PARSE_OK = 0, // �����ɹ�
    LEPT_PARSE_EXPECT_VALUE, // ����ֵ��δ�ҵ�
    LEPT_PARSE_INVALID_VALUE, // ��Ч��ֵ
    LEPT_PARSE_ROOT_NOT_SINGULAR, // ���ڵ㲻�ǵ�һֵ
    LEPT_PARSE_NUMBER_TOO_BIG, // ����̫���޷�����
    LEPT_PARSE_MISS_QUOTATION_MARK, // ȱ������
    LEPT_PARSE_INVALID_STRING_ESCAPE, // ��Ч���ַ���ת���ַ�
    LEPT_PARSE_INVALID_STRING_CHAR, // ��Ч���ַ����ַ�
    LEPT_PARSE_INVALID_UNICODE_HEX, // ��Ч�� Unicode ʮ������ֵ
    LEPT_PARSE_INVALID_UNICODE_SURROGATE, // ��Ч�� Unicode ����ַ�
    LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, // ȱ�ٶ��Ż�����
    LEPT_PARSE_MISS_KEY, // ȱ�ټ�
    LEPT_PARSE_MISS_COLON, // ȱ��ð��
    LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET // ȱ�ٶ��Ż������
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

size_t lept_get_array_size(const lept_value* v);
lept_value* lept_get_array_element(const lept_value* v, size_t index);

size_t lept_get_object_size(const lept_value* v); // ��ȡ�����Ա����
const char* lept_get_object_key(const lept_value* v, size_t index);
size_t lept_get_object_key_length(const lept_value* v, size_t index);
lept_value* lept_get_object_value(const lept_value* v, size_t index);


#endif /* JSONPARSE_H__ */