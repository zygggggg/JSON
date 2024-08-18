#ifndef JSONPARSE_H__
#define JSONPARSE_H__

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

// ����JSONֵ�Ľṹ��
typedef struct {
    lept_type type;  // JSONֵ������
}lept_value;

// ���������������
enum {
    LEPT_PARSE_OK = 0,         // �����ɹ�
    LEPT_PARSE_EXPECT_VALUE,   // �����õ�ֵ
    LEPT_PARSE_INVALID_VALUE,  // ��Ч��ֵ
    LEPT_PARSE_ROOT_NOT_SINGULAR // ���ڵ㲻�ǵ�һ��
};

// ����JSON�ַ���
int lept_parse(lept_value* v, const char* json); 

// ��ȡ�������JSONֵ������
lept_type lept_get_type(const lept_value* v);  

#endif /* JSONPARSE_H__ */