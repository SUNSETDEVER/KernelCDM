#ifndef CDM_AST_H
#define CDM_AST_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CDM_TYPE_INT = 0,
    CDM_TYPE_FLOAT,
    CDM_TYPE_PTR,
    CDM_TYPE_STRUCT,
    CDM_TYPE_ARRAY,
    CDM_TYPE_STRING,
    CDM_TYPE_REF
} CDM_ValueType_t;

typedef struct {
    CDM_ValueType_t type;
    int             struct_id;
    int             array_len;
    int             array_dim;
    int             array_sizes[4];
    CDM_ValueType_t array_elem_type;
    int             array_elem_struct_id;
    int             owned;
    int             from_new;
    union {
        int64_t i;
        double  f;
        void*   p;
    } v;
} CDM_Value_t;

#define CDM_MAX_STRUCT_TYPES      16
#define CDM_MAX_FIELDS_PER_STRUCT 16

typedef struct {
    char            name[32];
    CDM_ValueType_t type;
    int             struct_id;
    int             array_len;
} CDM_FieldDef_t;

typedef struct {
    char            name[32];
    CDM_FieldDef_t  fields[CDM_MAX_FIELDS_PER_STRUCT];
    int             field_count;
} CDM_StructDef_t;

typedef enum {
    TOK_EOF = 0, TOK_IDENT, TOK_NUMBER, TOK_STRING,
    TOK_OP, TOK_PUNCT, TOK_KEYWORD
} TokenType_t;

typedef struct { TokenType_t type; char text[64]; int line; } Token_t;

typedef enum {
    AST_PROGRAM = 0,
    AST_FUNC_DECL, AST_PARAM,
    AST_VAR_DECL, AST_ARRAY_DECL, AST_STRUCT_DEF,
    AST_ENUM_DECL,                     /* ★ 新增 */
    AST_BLOCK, AST_RETURN, AST_EXPR_STMT,
    AST_IF, AST_WHILE, AST_FOR, AST_RANGE_FOR,
    AST_DO_WHILE,
    AST_SWITCH, AST_CASE, AST_DEFAULT,
    AST_BREAK, AST_CONTINUE,
    AST_TERNARY,
    AST_BINARY, AST_UNARY, AST_ASSIGN,
    AST_CAST,                          /* ★ 新增 */
    AST_SIZEOF,                        /* ★ 新增 */
    AST_FUNC_CALL, AST_METHOD_CALL,
    AST_IDENTIFIER,
    AST_NUMBER, AST_STRING, AST_MEMBER,
    AST_MEMBER_ACCESS, AST_ARRAY_SUBSCRIPT, AST_ARRAY_INIT,
    AST_NEW, AST_NEW_ARRAY, AST_DELETE, AST_DELETE_ARRAY
} ASTType_t;

typedef struct ASTNode {
    ASTType_t type;
    char      name[64];
    int64_t   ival;
    double    fval;
    int       is_ptr;
    int       is_auto;
    int       struct_id;
    CDM_ValueType_t decl_type;
    int       line;

    int             array_dim;
    int             array_len;
    int             array_sizes[4];

    void*  cached_symbol;
    void*  cached_host_func;
    void*  cached_host_ex;
    int    cached_sym_tried;
    int    cached_hf_tried;
    int    cached_he_tried;

    struct ASTNode* first_child;
    struct ASTNode* next_sibling;
    struct ASTNode* left;
    struct ASTNode* right;
    struct ASTNode* body;
    struct ASTNode* init;
    struct ASTNode* update;
    struct ASTNode* cond;
    struct ASTNode* step;

    struct ASTNode** args;
    int              arg_count;
    struct ASTNode** elems;
    int              elem_count;
} ASTNode_t;

typedef struct { const char* src; int pos; int len; int line; Token_t cur; } CDM_Parser_t;

#define CDM_MAX_ERRORS 16
typedef struct { int line; int col; char message[80]; } CDM_Error_t;

void               CDM_ClearErrors(void);
int                CDM_GetErrorCount(void);
const CDM_Error_t* CDM_GetError(int idx);

ASTNode_t* CDM_Parse(const char* source);
void       CDM_FreeAST(ASTNode_t* root);
void       CDM_PrintAST(ASTNode_t* node, int indent);
int        CDM_Execute(ASTNode_t* root);

typedef int (*CDM_HostFunc_t)(int argc, const char** argv);
void CDM_RegisterHostFunc(const char* name, CDM_HostFunc_t fn);
void CDM_RegisterHostFuncFloat(const char* name, CDM_HostFunc_t fn);

typedef CDM_Value_t (*CDM_HostFuncEx_t)(int argc, CDM_Value_t* argv);
void CDM_RegisterHostFuncEx(const char* name, CDM_HostFuncEx_t fn);

CDM_Value_t CDM_MakeString(const char* s);
CDM_Value_t CDM_MakeStringOwned(char* s);
CDM_Value_t CDM_MakePtr(void* p, int owned);

int          CDM_IsRef(CDM_Value_t v);
CDM_Value_t  CDM_Deref(CDM_Value_t v);
CDM_Value_t* CDM_RefPtr(CDM_Value_t v);
void         CDM_AssignRef(CDM_Value_t ref, CDM_Value_t value);

#ifdef __cplusplus
}
#endif

#endif