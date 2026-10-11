#include "CDM_AST.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>

static int64_t cdm_atoll(const char* s) {
  if (!s) return 0;
  int64_t r = 0;
  int sign = 1;
  while (*s == ' ' || *s == '\t') s++;
  if (*s == '-') { sign = -1; s++; }
  else if (*s == '+') s++;
  while (*s >= '0' && *s <= '9') {
    r = r * 10 + (*s - '0');
    s++;
  }
  return sign * r;
}

static int64_t cdm_strtoll(const char* s, int base) {
  if (!s) return 0;
  int64_t r = 0;
  int sign = 1;
  int auto_base = (base == 0);
  while (*s == ' ' || *s == '\t') s++;
  if (*s == '-') { sign = -1; s++; }
  else if (*s == '+') s++;
  if (auto_base) {
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) { base = 16; s += 2; }
    else if (s[0] == '0') { base = 8; s++; }
    else base = 10;
  } else if (base == 16) {
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s += 2;
  }
  while (*s) {
    int d;
    if (*s >= '0' && *s <= '9') d = *s - '0';
    else if (*s >= 'a' && *s <= 'f') d = *s - 'a' + 10;
    else if (*s >= 'A' && *s <= 'F') d = *s - 'A' + 10;
    else break;
    if (d >= base) break;
    r = r * base + d;
    s++;
  }
  return sign * r;
}

#define CDM_MAX_HOST_FUNCS 128
#define CDM_MAX_HOST_EX 64
#define MAX_SYMBOLS 256
#define MAX_SCOPE_DEPTH 32
#define MAX_USER_FUNCS 64
#define CDM_ARR_MAGIC 0xDEADBEEF

typedef struct {
  char name[64];
  CDM_HostFunc_t func;
} HostFuncEntry_t;
typedef struct {
  char name[64];
  CDM_HostFuncEx_t func;
} HostExEntry_t;
typedef struct {
  char name[64];
  CDM_Value_t value;
  int in_use;
} Symbol_t;

static HostFuncEntry_t* g_host_funcs = NULL;
static char* g_float_funcs = NULL;
static HostExEntry_t* g_host_ex = NULL;
static CDM_StructDef_t* g_struct_defs = NULL;
static Symbol_t* g_symbols = NULL;
static ASTNode_t** g_user_funcs = NULL;

static int g_host_func_count = 0;
static int g_float_func_count = 0;
static int g_host_ex_count = 0;
static int g_struct_def_count = 0;
static int g_symbol_count = 0;
static int g_user_func_count = 0;

static int g_scope_mark[MAX_SCOPE_DEPTH];
static int g_scope_depth = 0;

static int g_cdm_allocated = 0;

static int g_break_flag = 0;
static int g_continue_flag = 0;

static int g_parse_line = 1;
static int g_current_line = 0;

static int g_panic_mode = 0;

#define MAX_ENUM_ENTRIES 128
typedef struct {
  char    name[64];
  int64_t value;
} EnumEntry_t;
static EnumEntry_t g_enums[MAX_ENUM_ENTRIES];
static int g_enum_count = 0;

static int find_enum(const char* name) {
  for (int i = 0; i < g_enum_count; i++)
    if (strcmp(g_enums[i].name, name) == 0) return i;
  return -1;
}

static int cdm_ensure_alloc(void) {
  if (g_cdm_allocated) return 1;
  g_host_funcs = (HostFuncEntry_t*)calloc(CDM_MAX_HOST_FUNCS, sizeof(HostFuncEntry_t));
  g_float_funcs = (char*)calloc(CDM_MAX_HOST_FUNCS, 64);
  g_host_ex = (HostExEntry_t*)calloc(CDM_MAX_HOST_EX, sizeof(HostExEntry_t));
  g_struct_defs = (CDM_StructDef_t*)calloc(CDM_MAX_STRUCT_TYPES, sizeof(CDM_StructDef_t));
  g_symbols = (Symbol_t*)calloc(MAX_SYMBOLS, sizeof(Symbol_t));
  g_user_funcs = (ASTNode_t**)calloc(MAX_USER_FUNCS, sizeof(ASTNode_t*));
  if (!g_host_funcs || !g_float_funcs || !g_host_ex || !g_struct_defs || !g_symbols || !g_user_funcs)
    return 0;
  g_cdm_allocated = 1;
  return 1;
}

static const char* float_func_at(int i) { return g_float_funcs + i * 64; }

void CDM_RegisterHostFunc(const char* name, CDM_HostFunc_t fn) {
  if (!cdm_ensure_alloc()) return;
  if (g_host_func_count >= CDM_MAX_HOST_FUNCS) return;
  strncpy(g_host_funcs[g_host_func_count].name, name, 63);
  g_host_funcs[g_host_func_count].name[63] = '\0';
  g_host_funcs[g_host_func_count].func = fn;
  g_host_func_count++;
}
void CDM_RegisterHostFuncFloat(const char* name, CDM_HostFunc_t fn) {
  if (!cdm_ensure_alloc()) return;
  if (g_host_func_count >= CDM_MAX_HOST_FUNCS) return;
  strncpy(g_host_funcs[g_host_func_count].name, name, 63);
  g_host_funcs[g_host_func_count].name[63] = '\0';
  g_host_funcs[g_host_func_count].func = fn;
  g_host_func_count++;
  if (g_float_func_count < CDM_MAX_HOST_FUNCS) {
    strncpy(g_float_funcs + g_float_func_count * 64, name, 63);
    g_float_funcs[g_float_func_count * 64 + 63] = '\0';
    g_float_func_count++;
  }
}
void CDM_RegisterHostFuncEx(const char* name, CDM_HostFuncEx_t fn) {
  if (!cdm_ensure_alloc()) return;
  if (g_host_ex_count >= CDM_MAX_HOST_EX) return;
  strncpy(g_host_ex[g_host_ex_count].name, name, 63);
  g_host_ex[g_host_ex_count].name[63] = '\0';
  g_host_ex[g_host_ex_count].func = fn;
  g_host_ex_count++;
}

static CDM_HostFunc_t find_host_func(const char* name) {
  if (!g_host_funcs) return NULL;
  for (int i = 0; i < g_host_func_count; i++)
    if (strcmp(g_host_funcs[i].name, name) == 0) return g_host_funcs[i].func;
  return NULL;
}
static CDM_HostFuncEx_t find_host_ex(const char* name) {
  if (!g_host_ex) return NULL;
  for (int i = 0; i < g_host_ex_count; i++)
    if (strcmp(g_host_ex[i].name, name) == 0) return g_host_ex[i].func;
  return NULL;
}
static int is_float_func(const char* name) {
  if (!g_float_funcs) return 0;
  for (int i = 0; i < g_float_func_count; i++)
    if (strcmp(float_func_at(i), name) == 0) return 1;
  return 0;
}

static int find_struct_def(const char* name) {
  if (!g_struct_defs) return -1;
  for (int i = 0; i < g_struct_def_count; i++)
    if (strcmp(g_struct_defs[i].name, name) == 0) return i;
  return -1;
}
static int find_field_index(int sid, const char* name) {
  if (sid < 0 || sid >= g_struct_def_count) return -1;
  CDM_StructDef_t* s = &g_struct_defs[sid];
  for (int i = 0; i < s->field_count; i++)
    if (strcmp(s->fields[i].name, name) == 0) return i;
  return -1;
}
static int is_user_type(const char* name) { return find_struct_def(name) >= 0; }

static CDM_Error_t g_errors[CDM_MAX_ERRORS];
static int g_error_count = 0;
static int g_error_overflow = 0;

void CDM_ClearErrors(void) {
  g_error_count = 0;
  g_error_overflow = 0;
  g_panic_mode = 0;
}
int CDM_GetErrorCount(void) { return g_error_count + (g_error_overflow ? 1 : 0); }
const CDM_Error_t* CDM_GetError(int idx) {
  if (idx < 0 || idx >= g_error_count) return NULL;
  return &g_errors[idx];
}

static void report_error(CDM_Parser_t* p, const char* fmt, ...) {
  if (g_error_count >= CDM_MAX_ERRORS) { g_error_overflow = 1; return; }
  CDM_Error_t* e = &g_errors[g_error_count++];
  e->line = p ? p->line : g_current_line;
  e->col = p ? p->pos : 0;
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(e->message, sizeof(e->message), fmt, ap);
  va_end(ap);
}

static int64_t sizeof_value(CDM_Value_t v) {
  switch (v.type) {
    case CDM_TYPE_FLOAT:  return 8;
    case CDM_TYPE_INT:    return 4;
    case CDM_TYPE_PTR:    return 8;
    case CDM_TYPE_STRING: return 8;
    case CDM_TYPE_ARRAY: {
      int64_t es = 4;
      if (v.array_elem_type == CDM_TYPE_FLOAT) es = 8;
      else if (v.array_elem_type == CDM_TYPE_PTR) es = 8;
      return es * v.array_len;
    }
    case CDM_TYPE_STRUCT: {
      if (v.struct_id < 0) return 0;
      int64_t total = 0;
      CDM_StructDef_t* sd = &g_struct_defs[v.struct_id];
      for (int i = 0; i < sd->field_count; i++) {
        int64_t es = 4;
        if (sd->fields[i].type == CDM_TYPE_FLOAT) es = 8;
        else if (sd->fields[i].type == CDM_TYPE_PTR) es = 8;
        if (sd->fields[i].array_len > 0) total += es * sd->fields[i].array_len;
        else total += es;
      }
      return total;
    }
    default: return 4;
  }
}

static int64_t eval_const(ASTNode_t* node) {
  if (!node) return 0;
  if (node->type == AST_NUMBER) return node->ival;
  if (node->type == AST_IDENTIFIER) {
    int i = find_enum(node->name);
    return (i >= 0) ? g_enums[i].value : 0;
  }
  if (node->type == AST_UNARY && strcmp(node->name, "-") == 0)
    return -eval_const(node->left);
  if (node->type == AST_UNARY && strcmp(node->name, "~") == 0)
    return ~eval_const(node->left);
  if (node->type == AST_UNARY && strcmp(node->name, "!") == 0)
    return !eval_const(node->left);
  if (node->type == AST_BINARY) {
    int64_t l = eval_const(node->left);
    int64_t r = eval_const(node->right);
    const char* op = node->name;
    if (strcmp(op, "+") == 0)  return l + r;
    if (strcmp(op, "-") == 0)  return l - r;
    if (strcmp(op, "*") == 0)  return l * r;
    if (strcmp(op, "/") == 0)  return r != 0 ? l / r : 0;
    if (strcmp(op, "%") == 0)  return r != 0 ? l % r : 0;
    if (strcmp(op, "&") == 0)  return l & r;
    if (strcmp(op, "|") == 0)  return l | r;
    if (strcmp(op, "^") == 0)  return l ^ r;
    if (strcmp(op, "<<") == 0) return l << r;
    if (strcmp(op, ">>") == 0) return l >> r;
  }
  return 0;
}

static void init_common(CDM_Value_t* v) {
  v->struct_id = -1;
  v->array_len = 0;
  v->array_dim = 0;
  v->array_sizes[0] = v->array_sizes[1] = v->array_sizes[2] = v->array_sizes[3] = 0;
  v->array_elem_type = CDM_TYPE_INT;
  v->array_elem_struct_id = -1;
  v->owned = 0;
  v->from_new = 0;
  v->v.p = NULL;
}
static CDM_Value_t make_int(int64_t i) {
  CDM_Value_t v; init_common(&v);
  v.type = CDM_TYPE_INT; v.v.i = i; return v;
}
static CDM_Value_t make_float(double f) {
  CDM_Value_t v; init_common(&v);
  v.type = CDM_TYPE_FLOAT; v.v.f = f; return v;
}
static CDM_Value_t make_ptr(void* p, int owned) {
  CDM_Value_t v; init_common(&v);
  v.type = CDM_TYPE_PTR; v.owned = owned; v.v.p = p; return v;
}
static CDM_Value_t make_struct(void* p, int sid, int owned) {
  CDM_Value_t v; init_common(&v);
  v.type = CDM_TYPE_STRUCT; v.struct_id = sid; v.owned = owned; v.v.p = p; return v;
}
static CDM_Value_t make_array(void* p, int len, int dim,
                              const int* sizes, CDM_ValueType_t elem, int esid, int owned) {
  CDM_Value_t v; init_common(&v);
  v.type = CDM_TYPE_ARRAY;
  v.array_len = len;
  v.array_dim = dim;
  for (int i = 0; i < 4; i++) v.array_sizes[i] = (i < dim) ? sizes[i] : 0;
  v.array_elem_type = elem;
  v.array_elem_struct_id = esid;
  v.owned = owned;
  v.v.p = p;
  return v;
}
static CDM_Value_t make_ref(CDM_Value_t* ptr) {
  CDM_Value_t v; init_common(&v);
  v.type = CDM_TYPE_REF; v.v.p = ptr; return v;
}

CDM_Value_t CDM_MakeString(const char* s) {
  CDM_Value_t v; init_common(&v);
  v.type = CDM_TYPE_STRING;
  size_t n = s ? strlen(s) : 0;
  char* buf = (char*)malloc(n + 1);
  if (!buf) return v;
  if (s) memcpy(buf, s, n);
  buf[n] = '\0';
  v.owned = 1;
  v.v.p = buf;
  return v;
}
CDM_Value_t CDM_MakeStringOwned(char* s) {
  CDM_Value_t v; init_common(&v);
  v.type = CDM_TYPE_STRING; v.owned = 1; v.v.p = s; return v;
}
CDM_Value_t CDM_MakePtr(void* p, int owned) { return make_ptr(p, owned); }

int CDM_IsRef(CDM_Value_t v) { return v.type == CDM_TYPE_REF; }
CDM_Value_t* CDM_RefPtr(CDM_Value_t v) {
  return (v.type == CDM_TYPE_REF && v.v.p) ? (CDM_Value_t*)v.v.p : NULL;
}
CDM_Value_t CDM_Deref(CDM_Value_t v) {
  int guard = 0;
  while (v.type == CDM_TYPE_REF && v.v.p && guard < 8) {
    v = *(CDM_Value_t*)v.v.p;
    guard++;
  }
  return v;
}

static void value_release(CDM_Value_t v);

void CDM_AssignRef(CDM_Value_t ref, CDM_Value_t value) {
  CDM_Value_t* slot = CDM_RefPtr(ref);
  if (!slot) return;

  if (value.type == CDM_TYPE_STRING) {
    if (slot->owned && slot->v.p) free(slot->v.p);
    const char* s = value.v.p ? (const char*)value.v.p : "";
    size_t n = strlen(s);
    char* buf = (char*)malloc(n + 1);
    if (buf) { memcpy(buf, s, n); buf[n] = '\0'; }
    CDM_Value_t nv;
    init_common(&nv);
    nv.type = CDM_TYPE_STRING;
    nv.owned = 1;
    nv.v.p = buf;
    *slot = nv;
    return;
  }

  value_release(*slot);
  *slot = value;
  slot->owned = 0;
  slot->from_new = 0;
}

static int val_is_float(CDM_Value_t v) { return v.type == CDM_TYPE_FLOAT; }
static int val_is_ptr(CDM_Value_t v)   { return v.type == CDM_TYPE_PTR; }
static int val_is_struct(CDM_Value_t v){ return v.type == CDM_TYPE_STRUCT; }
static int val_is_array(CDM_Value_t v) { return v.type == CDM_TYPE_ARRAY; }
static int val_is_string(CDM_Value_t v){ return v.type == CDM_TYPE_STRING; }

static double val_to_double_raw(CDM_Value_t v) {
  if (v.type == CDM_TYPE_FLOAT) return v.v.f;
  if (v.type == CDM_TYPE_INT) return (double)v.v.i;
  if (v.type == CDM_TYPE_STRING && v.v.p) return atof((const char*)v.v.p);
  return 0.0;
}
static int64_t val_to_int_raw(CDM_Value_t v) {
  if (v.type == CDM_TYPE_FLOAT) return (int64_t)v.v.f;
  if (v.type == CDM_TYPE_PTR) return (int64_t)(intptr_t)v.v.p;
  if (v.type == CDM_TYPE_STRING && v.v.p) return cdm_atoll((const char*)v.v.p);
  return v.v.i;
}
static const char* val_to_str_raw(CDM_Value_t v) {
  if (v.type == CDM_TYPE_STRING && v.v.p) return (const char*)v.v.p;
  return "";
}
static double val_to_double(CDM_Value_t v) { return val_to_double_raw(CDM_Deref(v)); }
static int64_t val_to_int(CDM_Value_t v)   { return val_to_int_raw(CDM_Deref(v)); }

static void value_release(CDM_Value_t v) {
  if (v.type == CDM_TYPE_REF) return;
  if (!v.v.p) return;
  if (v.type == CDM_TYPE_STRING && v.owned) { free(v.v.p); return; }
  if (v.type == CDM_TYPE_ARRAY && v.owned) {
    CDM_Value_t* elems = (CDM_Value_t*)v.v.p;
    for (int i = 0; i < v.array_len; i++) value_release(elems[i]);
    free(elems);
    return;
  }
  if (v.owned) free(v.v.p);
}

typedef struct {
  uint32_t magic;
  uint32_t count;
  uint32_t elem_type;
  int32_t elem_struct_id;
} CDM_ArrayHeader_t;

static CDM_Value_t* array_new(int n, CDM_ValueType_t et, int esid) {
  size_t total = sizeof(CDM_ArrayHeader_t) + n * sizeof(CDM_Value_t);
  uint8_t* raw = (uint8_t*)malloc(total);
  if (!raw) return NULL;
  CDM_ArrayHeader_t* h = (CDM_ArrayHeader_t*)raw;
  h->magic = CDM_ARR_MAGIC;
  h->count = (uint32_t)n;
  h->elem_type = (uint32_t)et;
  h->elem_struct_id = esid;
  CDM_Value_t* arr = (CDM_Value_t*)(raw + sizeof(CDM_ArrayHeader_t));
  for (int i = 0; i < n; i++) {
    if (et == CDM_TYPE_STRUCT && esid >= 0) {
      int nf = g_struct_defs[esid].field_count;
      CDM_Value_t* fblock = (CDM_Value_t*)calloc(nf, sizeof(CDM_Value_t));
      for (int j = 0; j < nf; j++) fblock[j] = make_int(0);
      arr[i] = make_struct(fblock, esid, 1);
    } else if (et == CDM_TYPE_FLOAT) {
      arr[i] = make_float(0.0);
    } else if (et == CDM_TYPE_STRING) {
      arr[i] = CDM_MakeString("");
    } else {
      arr[i] = make_int(0);
    }
  }
  return arr;
}
static int array_free(void* p) {
  if (!p) return 0;
  uint8_t* raw = (uint8_t*)p - sizeof(CDM_ArrayHeader_t);
  CDM_ArrayHeader_t* h = (CDM_ArrayHeader_t*)raw;
  if (h->magic != CDM_ARR_MAGIC) return 0;
  CDM_Value_t* arr = (CDM_Value_t*)p;
  for (uint32_t i = 0; i < h->count; i++) value_release(arr[i]);
  free(raw);
  return 1;
}

static void scope_enter(void) {
  if (g_scope_depth < MAX_SCOPE_DEPTH) {
    g_scope_mark[g_scope_depth] = g_symbol_count;
    g_scope_depth++;
  }
}
static void scope_exit(void) {
  if (g_scope_depth > 0) {
    g_scope_depth--;
    int mark = g_scope_mark[g_scope_depth];
    for (int i = mark; i < g_symbol_count; i++) {
      if (g_symbols[i].in_use) value_release(g_symbols[i].value);
      g_symbols[i].in_use = 0;
    }
    g_symbol_count = mark;
  }
}
static Symbol_t* find_symbol(const char* name) {
  if (!g_symbols) return NULL;
  for (int i = g_symbol_count - 1; i >= 0; i--)
    if (g_symbols[i].in_use && strcmp(g_symbols[i].name, name) == 0) return &g_symbols[i];
  return NULL;
}
static Symbol_t* create_symbol(const char* name) {
  if (!g_symbols) return NULL;
  if (g_symbol_count >= MAX_SYMBOLS) return NULL;
  Symbol_t* s = &g_symbols[g_symbol_count++];
  strncpy(s->name, name, 63);
  s->name[63] = '\0';
  s->value = make_int(0);
  s->in_use = 1;
  return s;
}

static void collect_user_funcs(ASTNode_t* program) {
  g_user_func_count = 0;
  ASTNode_t* c = program->first_child;
  while (c) {
    if (c->type == AST_FUNC_DECL && g_user_func_count < MAX_USER_FUNCS)
      g_user_funcs[g_user_func_count++] = c;
    c = c->next_sibling;
  }
}
static ASTNode_t* find_user_func(const char* name) {
  for (int i = 0; i < g_user_func_count; i++)
    if (strcmp(g_user_funcs[i]->name, name) == 0) return g_user_funcs[i];
  return NULL;
}

static void clear_ast_cache(ASTNode_t* node) {
  if (!node) return;
  node->cached_symbol = NULL;
  node->cached_host_func = NULL;
  node->cached_host_ex = NULL;
  node->cached_sym_tried = 0;
  node->cached_hf_tried = 0;
  node->cached_he_tried = 0;
  ASTNode_t* c = node->first_child;
  while (c) { clear_ast_cache(c); c = c->next_sibling; }
  if (node->left)   clear_ast_cache(node->left);
  if (node->right)  clear_ast_cache(node->right);
  if (node->body)   clear_ast_cache(node->body);
  if (node->init)   clear_ast_cache(node->init);
  if (node->update) clear_ast_cache(node->update);
  if (node->cond)   clear_ast_cache(node->cond);
  if (node->step)   clear_ast_cache(node->step);
  for (int i = 0; i < node->arg_count; i++)  clear_ast_cache(node->args[i]);
  for (int i = 0; i < node->elem_count; i++) clear_ast_cache(node->elems[i]);
}

static const char* KEYWORDS[] = {
  "int", "char", "void", "float", "double", "return",
  "if", "else", "while", "for", "break", "continue",
  "switch", "case", "default", "do",
  "enum", "sizeof",
  "const", "static", "long", "short", "unsigned", "signed",
  "struct", "class", "true", "false", "nullptr",
  "new", "delete", "auto",
  "int8_t", "uint8_t", "int16_t", "uint16_t",
  "int32_t", "uint32_t", "int64_t", "uint64_t",
  "size_t", "ssize_t", "intptr_t", "uintptr_t",
  "intmax_t", "uintmax_t",
  "int_fast8_t", "uint_fast8_t", "int_fast16_t", "uint_fast16_t",
  "int_fast32_t", "uint_fast32_t", "int_fast64_t", "uint_fast64_t",
  "int_least8_t", "uint_least8_t", "int_least16_t", "uint_least16_t",
  "int_least32_t", "uint_least32_t", "int_least64_t", "uint_least64_t"
};
#define NUM_KEYWORDS (sizeof(KEYWORDS) / sizeof(KEYWORDS[0]))
static int is_keyword(const char* s) {
  for (int i = 0; i < (int)NUM_KEYWORDS; i++)
    if (strcmp(s, KEYWORDS[i]) == 0) return 1;
  return 0;
}
static int is_ident_start(char c) { return isalpha((unsigned char)c) || c == '_'; }
static int is_ident_char(char c)  { return isalnum((unsigned char)c) || c == '_'; }

static void next_token(CDM_Parser_t* p) {
  while (p->pos < p->len) {
    char c = p->src[p->pos];
    if (c == ' ' || c == '\t' || c == '\r') { p->pos++; continue; }
    if (c == '\n') { p->line++; p->pos++; continue; }
    if (c == '/' && p->pos + 1 < p->len && p->src[p->pos + 1] == '/') {
      while (p->pos < p->len && p->src[p->pos] != '\n') p->pos++;
      continue;
    }
    if (c == '/' && p->pos + 1 < p->len && p->src[p->pos + 1] == '*') {
      p->pos += 2;
      while (p->pos + 1 < p->len && !(p->src[p->pos] == '*' && p->src[p->pos + 1] == '/')) {
        if (p->src[p->pos] == '\n') p->line++;
        p->pos++;
      }
      p->pos += 2;
      continue;
    }
    break;
  }
  g_parse_line = p->line;

  if (p->pos >= p->len) {
    p->cur.type = TOK_EOF;
    p->cur.text[0] = '\0';
    return;
  }
  char c = p->src[p->pos];

  if (c == '"') {
    p->pos++;
    int j = 0;
    while (p->pos < p->len && p->src[p->pos] != '"' && j < 63) {
      char ch = p->src[p->pos];
      if (ch == '\\' && p->pos + 1 < p->len) {
        p->pos++;
        char esc = p->src[p->pos];
        if (esc == 'n') ch = '\n';
        else if (esc == 't') ch = '\t';
        else if (esc == '\\') ch = '\\';
        else if (esc == '"') ch = '"';
        else ch = esc;
      }
      p->cur.text[j++] = ch;
      p->pos++;
    }
    p->cur.text[j] = '\0';
    if (p->pos < p->len) p->pos++;
    p->cur.type = TOK_STRING;
    return;
  }

  if (isdigit((unsigned char)c) || (c == '.' && p->pos + 1 < p->len && isdigit((unsigned char)p->src[p->pos + 1]))) {
    int j = 0;
    if (c == '0' && p->pos + 1 < p->len && (p->src[p->pos + 1] == 'x' || p->src[p->pos + 1] == 'X')) {
      p->cur.text[j++] = p->src[p->pos++];
      p->cur.text[j++] = p->src[p->pos++];
      while (p->pos < p->len && isxdigit((unsigned char)p->src[p->pos]) && j < 63)
        p->cur.text[j++] = p->src[p->pos++];
    } else {
      while (p->pos < p->len && isdigit((unsigned char)p->src[p->pos]) && j < 63)
        p->cur.text[j++] = p->src[p->pos++];
      if (p->pos < p->len && p->src[p->pos] == '.') {
        p->cur.text[j++] = p->src[p->pos++];
        while (p->pos < p->len && isdigit((unsigned char)p->src[p->pos]) && j < 63)
          p->cur.text[j++] = p->src[p->pos++];
      }
      if (p->pos < p->len && (p->src[p->pos] == 'e' || p->src[p->pos] == 'E')) {
        p->cur.text[j++] = p->src[p->pos++];
        if (p->pos < p->len && (p->src[p->pos] == '+' || p->src[p->pos] == '-'))
          p->cur.text[j++] = p->src[p->pos++];
        while (p->pos < p->len && isdigit((unsigned char)p->src[p->pos]) && j < 63)
          p->cur.text[j++] = p->src[p->pos++];
      }
    }

    while (p->pos < p->len) {
      char sfx = p->src[p->pos];
      if (sfx == 'f' || sfx == 'F' || sfx == 'l' || sfx == 'L' ||
          sfx == 'u' || sfx == 'U') {
        p->pos++;
        continue;
      }
      break;
    }

    p->cur.text[j] = '\0';
    p->cur.type = TOK_NUMBER;
    return;
  }

  if (is_ident_start(c)) {
    int j = 0;
    while (p->pos < p->len && is_ident_char(p->src[p->pos]) && j < 63)
      p->cur.text[j++] = p->src[p->pos++];
    p->cur.text[j] = '\0';
    p->cur.type = is_keyword(p->cur.text) ? TOK_KEYWORD : TOK_IDENT;
    return;
  }

  static const char* TWO[] = {
    "::", "<<", ">>", "==", "!=", "<=", ">=", "&&", "||",
    "++", "--", "->", "+=", "-=", "*=", "/=",
    "%=", "&=", "|=", "^="
  };
  if (p->pos + 3 < p->len) {
    char four[5] = { p->src[p->pos], p->src[p->pos+1], p->src[p->pos+2], p->src[p->pos+3], '\0' };
    if (strcmp(four, "<<==") == 0 || strcmp(four, ">>==") == 0) {
      strcpy(p->cur.text, four);
      p->cur.type = TOK_OP;
      p->pos += 4;
      return;
    }
  }
  if (p->pos + 2 < p->len) {
    char three[4] = { p->src[p->pos], p->src[p->pos+1], p->src[p->pos+2], '\0' };
    if (strcmp(three, "<<=") == 0 || strcmp(three, ">>=") == 0) {
      strcpy(p->cur.text, three);
      p->cur.type = TOK_OP;
      p->pos += 3;
      return;
    }
  }
  if (p->pos + 1 < p->len) {
    char two[3] = { p->src[p->pos], p->src[p->pos + 1], '\0' };
    for (int i = 0; i < (int)(sizeof(TWO) / sizeof(TWO[0])); i++) {
      if (strcmp(two, TWO[i]) == 0) {
        strcpy(p->cur.text, two);
        p->cur.type = TOK_OP;
        p->pos += 2;
        return;
      }
    }
  }
  p->cur.text[0] = c;
  p->cur.text[1] = '\0';
  if (strchr("+-*/%=<>!&|^~?:.", c)) p->cur.type = TOK_OP;
  else p->cur.type = TOK_PUNCT;
  p->pos++;
}

static ASTNode_t* new_node(ASTType_t type) {
  ASTNode_t* n = (ASTNode_t*)calloc(1, sizeof(ASTNode_t));
  if (!n) return NULL;
  n->type = type;
  n->struct_id = -1;
  n->line = g_parse_line;
  return n;
}
static void add_child(ASTNode_t* parent, ASTNode_t* child) {
  if (!parent || !child) return;
  if (child->next_sibling) {
    printf("[AST] WARN: child already has sibling!\n");
    return;
  }
  if (!parent->first_child) parent->first_child = child;
  else {
    ASTNode_t* s = parent->first_child;
    while (s->next_sibling) s = s->next_sibling;
    s->next_sibling = child;
  }
}

static ASTNode_t* parse_expression(CDM_Parser_t* p);
static ASTNode_t* parse_statement(CDM_Parser_t* p);
static ASTNode_t* parse_block(CDM_Parser_t* p);
static ASTNode_t* parse_declaration(CDM_Parser_t* p, int consume_semi);
static ASTNode_t* parse_struct_def(CDM_Parser_t* p);
static ASTNode_t* parse_enum(CDM_Parser_t* p);

static int match_op(CDM_Parser_t* p, const char* op) {
  if (p->cur.type == TOK_OP && strcmp(p->cur.text, op) == 0) { next_token(p); return 1; }
  return 0;
}
static int match_punct(CDM_Parser_t* p, const char* c) {
  if (p->cur.type == TOK_PUNCT && strcmp(p->cur.text, c) == 0) { next_token(p); return 1; }
  return 0;
}
static int match_kw(CDM_Parser_t* p, const char* kw) {
  if (p->cur.type == TOK_KEYWORD && strcmp(p->cur.text, kw) == 0) { next_token(p); return 1; }
  return 0;
}

static void require_punct(CDM_Parser_t* p, const char* c) {
  if (p->cur.type == TOK_PUNCT && strcmp(p->cur.text, c) == 0) {
    next_token(p);
  } else if (p->cur.type == TOK_EOF) {
    report_error(p, "expected '%s' at end of file", c);
    g_panic_mode = 1;
  } else {
    report_error(p, "expected '%s', got '%s'", c, p->cur.text);
    g_panic_mode = 1;
  }
}

static void require_op(CDM_Parser_t* p, const char* op) {
  if (p->cur.type == TOK_OP && strcmp(p->cur.text, op) == 0) {
    next_token(p);
  } else if (p->cur.type == TOK_EOF) {
    report_error(p, "expected '%s' at end of file", op);
    g_panic_mode = 1;
  } else {
    report_error(p, "expected '%s', got '%s'", op, p->cur.text);
    g_panic_mode = 1;
  }
}

static int require_identifier(CDM_Parser_t* p, char* out, int out_size) {
  if (p->cur.type == TOK_IDENT) {
    strncpy(out, p->cur.text, out_size - 1);
    out[out_size - 1] = '\0';
    next_token(p);
    return 1;
  }
  if (p->cur.type == TOK_EOF) {
    report_error(p, "expected identifier at end of file");
  } else {
    report_error(p, "expected identifier, got '%s'", p->cur.text);
  }
  g_panic_mode = 1;
  out[0] = '\0';
  return 0;
}

static void skip_to_sync(CDM_Parser_t* p) {
  int guard = 0;
  while (p->cur.type != TOK_EOF && guard < 256) {
    if (p->cur.type == TOK_PUNCT) {
      if (p->cur.text[0] == ';') return;
      if (p->cur.text[0] == '}') return;
    }
    next_token(p);
    guard++;
  }
}

static void recover_statement(CDM_Parser_t* p) {
  skip_to_sync(p);
  if (p->cur.type == TOK_PUNCT && p->cur.text[0] == ';') {
    next_token(p);
  }
  g_panic_mode = 0;
}

static int is_type_keyword(CDM_Parser_t* p) {
  if (p->cur.type != TOK_KEYWORD) return 0;
  const char* t = p->cur.text;
  if (strcmp(t, "auto") == 0) return 1;
  return (strcmp(t, "int") == 0 || strcmp(t, "char") == 0 || strcmp(t, "void") == 0 ||
          strcmp(t, "float") == 0 || strcmp(t, "double") == 0 || strcmp(t, "long") == 0 ||
          strcmp(t, "short") == 0 || strcmp(t, "unsigned") == 0 || strcmp(t, "signed") == 0 ||
          strcmp(t, "static") == 0 || strcmp(t, "const") == 0 ||
          strncmp(t, "int", 3) == 0 || strncmp(t, "uint", 4) == 0 ||
          strncmp(t, "size_t", 6) == 0 || strncmp(t, "ssize_t", 7) == 0);
}
static CDM_ValueType_t type_from_keyword(const char* kw) {
  if (strstr(kw, "float") || strstr(kw, "double")) return CDM_TYPE_FLOAT;
  return CDM_TYPE_INT;
}

static ASTNode_t* parse_args(CDM_Parser_t* p) {
  ASTNode_t* call = new_node(AST_FUNC_CALL);
  if (!match_punct(p, "(")) return call;
  ASTNode_t* args[32];
  int argc = 0;
  if (p->cur.type != TOK_PUNCT || p->cur.text[0] != ')') {
    while (argc < 32) {
      args[argc++] = parse_expression(p);
      if (!match_punct(p, ",")) break;
    }
  }
  require_punct(p, ")");
  call->arg_count = argc;
  if (argc > 0) {
    call->args = (ASTNode_t**)calloc(argc, sizeof(ASTNode_t*));
    for (int i = 0; i < argc; i++) call->args[i] = args[i];
  }
  return call;
}

static ASTNode_t* parse_new(CDM_Parser_t* p) {
  match_kw(p, "new");
  ASTNode_t* n = new_node(AST_NEW);
  if (p->cur.type != TOK_IDENT && !is_type_keyword(p)) {
    report_error(p, "expected type after 'new', got '%s'", p->cur.text);
    g_panic_mode = 1;
    return n;
  }
  strncpy(n->name, p->cur.text, 63);
  if (is_user_type(n->name)) {
    n->decl_type = CDM_TYPE_STRUCT;
    n->struct_id = find_struct_def(n->name);
  } else {
    n->decl_type = type_from_keyword(n->name);
  }
  next_token(p);
  if (p->cur.type == TOK_PUNCT && p->cur.text[0] == '[') {
    next_token(p);
    if (p->cur.type != TOK_PUNCT || p->cur.text[0] != ']') {
      n->left = parse_expression(p);
    }
    require_punct(p, "]");
    n->type = AST_NEW_ARRAY;
  }
  return n;
}

static ASTNode_t* parse_primary(CDM_Parser_t* p) {
  if (p->cur.type == TOK_NUMBER) {
    ASTNode_t* n = new_node(AST_NUMBER);
    strncpy(n->name, p->cur.text, 63);
    if (strchr(p->cur.text, '.') || strchr(p->cur.text, 'e') || strchr(p->cur.text, 'E'))
      n->fval = atof(p->cur.text);
    else
      n->ival = cdm_strtoll(p->cur.text, 0);
    next_token(p);
    return n;
  }
  if (p->cur.type == TOK_STRING) {
    ASTNode_t* n = new_node(AST_STRING);
    strncpy(n->name, p->cur.text, 63);
    next_token(p);
    return n;
  }

  if (p->cur.type == TOK_KEYWORD && strcmp(p->cur.text, "sizeof") == 0) {
    next_token(p);
    if (!match_punct(p, "(")) {
      report_error(p, "expected '(' after sizeof");
      g_panic_mode = 1;
    }
    if (is_type_keyword(p)) {
      const char* t = p->cur.text;
      int64_t sz = 4;
      if (strstr(t, "char") || strcmp(t, "int8_t") == 0 || strcmp(t, "uint8_t") == 0) sz = 1;
      else if (strstr(t, "short") || strstr(t, "int16")) sz = 2;
      else if (strstr(t, "long") || strstr(t, "int64")) sz = 8;
      else if (strstr(t, "float") || strstr(t, "double")) sz = 8;
      else sz = 4;
      next_token(p);
      while (is_type_keyword(p)) next_token(p);
      while (p->cur.type == TOK_OP && strcmp(p->cur.text, "*") == 0) {
        sz = 8;
        next_token(p);
      }
      require_punct(p, ")");
      ASTNode_t* n = new_node(AST_NUMBER);
      n->ival = sz;
      snprintf(n->name, 63, "%lld", (long long)sz);
      return n;
    }
    ASTNode_t* e = parse_expression(p);
    require_punct(p, ")");
    ASTNode_t* sz_node = new_node(AST_SIZEOF);
    sz_node->left = e;
    return sz_node;
  }

  if (p->cur.type == TOK_PUNCT && p->cur.text[0] == '(') {
    int saved_pos = p->pos;
    int saved_line = p->line;
    Token_t saved_tok = p->cur;
    next_token(p);
    if (p->cur.type == TOK_KEYWORD && is_type_keyword(p)) {
      CDM_ValueType_t target = type_from_keyword(p->cur.text);
      next_token(p);
      while (is_type_keyword(p)) next_token(p);
      int is_ptr = 0;
      while (p->cur.type == TOK_OP && strcmp(p->cur.text, "*") == 0) {
        is_ptr = 1;
        next_token(p);
      }
      require_punct(p, ")");
      ASTNode_t* cast = new_node(AST_CAST);
      cast->decl_type = is_ptr ? CDM_TYPE_PTR : target;
      cast->is_ptr = is_ptr;
      cast->left = parse_primary(p);
      return cast;
    }
    p->pos = saved_pos;
    p->line = saved_line;
    p->cur = saved_tok;
    next_token(p);
    ASTNode_t* n = parse_expression(p);
    require_punct(p, ")");
    return n;
  }

  if (p->cur.type == TOK_KEYWORD && strcmp(p->cur.text, "new") == 0)
    return parse_new(p);
  if (p->cur.type == TOK_KEYWORD) {
    if (strcmp(p->cur.text, "true") == 0) {
      ASTNode_t* n = new_node(AST_NUMBER); n->ival = 1; next_token(p); return n;
    }
    if (strcmp(p->cur.text, "false") == 0 || strcmp(p->cur.text, "nullptr") == 0) {
      ASTNode_t* n = new_node(AST_NUMBER); n->ival = 0; next_token(p); return n;
    }
  }
  if (p->cur.type == TOK_OP) {
    const char* op = p->cur.text;
    int is_unary = (strcmp(op, "-") == 0 || strcmp(op, "!") == 0 || strcmp(op, "~") == 0 ||
                    strcmp(op, "++") == 0 || strcmp(op, "--") == 0 ||
                    strcmp(op, "&") == 0 || strcmp(op, "*") == 0);
    if (is_unary) {
      char opc[8];
      strncpy(opc, op, 7); opc[7] = '\0';
      next_token(p);
      ASTNode_t* n = new_node(AST_UNARY);
      strncpy(n->name, opc, 63);
      n->is_ptr = 0;
      n->left = parse_primary(p);
      return n;
    }
  }

  if (p->cur.type == TOK_IDENT || p->cur.type == TOK_KEYWORD) {
    char name[64];
    strncpy(name, p->cur.text, 63);
    name[63] = '\0';
    next_token(p);

    ASTNode_t* base = new_node(AST_IDENTIFIER);
    strncpy(base->name, name, 63);
    int had_suffix = 0;

    while (1) {
      if (p->cur.type == TOK_PUNCT && p->cur.text[0] == '[') {
        next_token(p);
        ASTNode_t* idx = parse_expression(p);
        require_punct(p, "]");
        ASTNode_t* sub = new_node(AST_ARRAY_SUBSCRIPT);
        sub->left = base; sub->right = idx; base = sub;
        had_suffix = 1;
      } else if (p->cur.type == TOK_OP && strcmp(p->cur.text, ".") == 0) {
        next_token(p);
        if (p->cur.type != TOK_IDENT) {
          report_error(p, "expected member name after '.', got '%s'", p->cur.text);
          g_panic_mode = 1;
          break;
        }
        char member[64];
        strncpy(member, p->cur.text, 63);
        member[63] = '\0';
        next_token(p);
        if (p->cur.type == TOK_PUNCT && p->cur.text[0] == '(') {
          ASTNode_t* call = parse_args(p);
          strncpy(call->name, member, 63);
          call->type = AST_METHOD_CALL;
          call->left = base;
          base = call;
        } else {
          ASTNode_t* acc = new_node(AST_MEMBER_ACCESS);
          strncpy(acc->name, member, 63);
          acc->left = base;
          base = acc;
        }
        had_suffix = 1;
      }
      else if (p->cur.type == TOK_OP &&
               (strcmp(p->cur.text, "++") == 0 || strcmp(p->cur.text, "--") == 0)) {
        char opc[4];
        strncpy(opc, p->cur.text, 3);
        opc[3] = '\0';
        next_token(p);
        ASTNode_t* post = new_node(AST_UNARY);
        strncpy(post->name, opc, 63);
        post->is_ptr = 1;
        post->left = base;
        base = post;
        had_suffix = 1;
      } else {
        break;
      }
    }

    if (had_suffix) return base;

    if (p->cur.type == TOK_OP && strcmp(p->cur.text, "::") == 0) {
      next_token(p);
      char sub_name[64];
      strncpy(sub_name, p->cur.text, 63);
      sub_name[63] = '\0';
      next_token(p);
      if (p->cur.type == TOK_PUNCT && p->cur.text[0] == '(') {
        CDM_FreeAST(base);
        ASTNode_t* call = parse_args(p);
        snprintf(call->name, 64, "%s::%s", name, sub_name);
        return call;
      }
      ASTNode_t* m = new_node(AST_MEMBER);
      strncpy(m->name, name, 63);
      CDM_FreeAST(base);
      ASTNode_t* sub = new_node(AST_IDENTIFIER);
      strncpy(sub->name, sub_name, 63);
      add_child(m, sub);
      return m;
    }
    if (p->cur.type == TOK_PUNCT && p->cur.text[0] == '(') {
      ASTNode_t* call = parse_args(p);
      strncpy(call->name, name, 63);
      return call;
    }
    return base;
  }

  report_error(p, "unexpected token '%s' in expression", p->cur.text);
  g_panic_mode = 1;
  ASTNode_t* n = new_node(AST_NUMBER);
  n->ival = 0;
  next_token(p);
  return n;
}

static int precedence(const char* op) {
  if (strcmp(op, "=") == 0 || strcmp(op, "+=") == 0 || strcmp(op, "-=") == 0 ||
      strcmp(op, "*=") == 0 || strcmp(op, "/=") == 0 || strcmp(op, "%=") == 0 ||
      strcmp(op, "&=") == 0 || strcmp(op, "|=") == 0 || strcmp(op, "^=") == 0 ||
      strcmp(op, "<<=") == 0 || strcmp(op, ">>=") == 0) return 1;
  if (strcmp(op, "||") == 0) return 2;
  if (strcmp(op, "&&") == 0) return 3;
  if (strcmp(op, "|") == 0) return 4;
  if (strcmp(op, "^") == 0) return 5;
  if (strcmp(op, "&") == 0) return 6;
  if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0) return 7;
  if (strcmp(op, "<") == 0 || strcmp(op, ">") == 0 ||
      strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0) return 8;
  if (strcmp(op, "<<") == 0 || strcmp(op, ">>") == 0) return 9;
  if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0) return 10;
  if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0 || strcmp(op, "%") == 0) return 11;
  return 0;
}
static int is_right_assoc(const char* op) {
  return (strcmp(op, "=") == 0 || strcmp(op, "+=") == 0 || strcmp(op, "-=") == 0 ||
          strcmp(op, "*=") == 0 || strcmp(op, "/=") == 0 || strcmp(op, "%=") == 0 ||
          strcmp(op, "&=") == 0 || strcmp(op, "|=") == 0 || strcmp(op, "^=") == 0 ||
          strcmp(op, "<<=") == 0 || strcmp(op, ">>=") == 0);
}

static ASTNode_t* parse_binary(CDM_Parser_t* p, int min_prec) {
  ASTNode_t* left = parse_primary(p);
  while (p->cur.type == TOK_OP) {
    if (strcmp(p->cur.text, ".") == 0) break;
    if (strcmp(p->cur.text, "?") == 0) break;
    int prec = precedence(p->cur.text);
    if (prec == 0 || prec < min_prec) break;
    char op[8];
    strncpy(op, p->cur.text, 7); op[7] = '\0';
    next_token(p);
    int next_min = is_right_assoc(op) ? prec : prec + 1;
    ASTNode_t* right = parse_binary(p, next_min);
    ASTNode_t* bin = new_node(AST_BINARY);
    strncpy(bin->name, op, 63);
    bin->left = left;
    bin->right = right;
    left = bin;
  }
  return left;
}

static ASTNode_t* parse_expression(CDM_Parser_t* p) {
  ASTNode_t* cond = parse_binary(p, 1);
  if (p->cur.type == TOK_OP && strcmp(p->cur.text, "?") == 0) {
    next_token(p);
    ASTNode_t* then_e = parse_expression(p);
    require_op(p, ":");
    ASTNode_t* else_e = parse_expression(p);
    ASTNode_t* t = new_node(AST_TERNARY);
    t->left = cond;
    t->right = then_e;
    t->init = else_e;
    return t;
  }
  return cond;
}

static ASTNode_t* parse_if(CDM_Parser_t* p) {
  match_kw(p, "if");
  require_punct(p, "(");
  ASTNode_t* cond = parse_expression(p);
  require_punct(p, ")");
  ASTNode_t* node = new_node(AST_IF);
  node->left = cond;
  node->body = parse_statement(p);
  if (p->cur.type == TOK_KEYWORD && strcmp(p->cur.text, "else") == 0) {
    next_token(p);
    node->right = parse_statement(p);
  }
  return node;
}
static ASTNode_t* parse_while(CDM_Parser_t* p) {
  match_kw(p, "while");
  require_punct(p, "(");
  ASTNode_t* cond = parse_expression(p);
  require_punct(p, ")");
  ASTNode_t* node = new_node(AST_WHILE);
  node->left = cond;
  node->body = parse_statement(p);
  return node;
}

static ASTNode_t* parse_do_while(CDM_Parser_t* p) {
  match_kw(p, "do");
  ASTNode_t* node = new_node(AST_DO_WHILE);
  node->body = parse_statement(p);
  match_kw(p, "while");
  require_punct(p, "(");
  node->left = parse_expression(p);
  require_punct(p, ")");
  require_punct(p, ";");
  return node;
}

static ASTNode_t* parse_switch(CDM_Parser_t* p) {
  match_kw(p, "switch");
  require_punct(p, "(");
  ASTNode_t* cond = parse_expression(p);
  require_punct(p, ")");
  require_punct(p, "{");

  ASTNode_t* node = new_node(AST_SWITCH);
  node->left = cond;
  ASTNode_t* body = new_node(AST_BLOCK);
  node->body = body;

  while (p->cur.type != TOK_EOF &&
         !(p->cur.type == TOK_PUNCT && p->cur.text[0] == '}')) {
    if (p->cur.type == TOK_KEYWORD && strcmp(p->cur.text, "case") == 0) {
      next_token(p);
      ASTNode_t* c = new_node(AST_CASE);
      c->left = parse_expression(p);
      require_op(p, ":");
      while (p->cur.type != TOK_EOF) {
        if (p->cur.type == TOK_KEYWORD &&
            (strcmp(p->cur.text, "case") == 0 || strcmp(p->cur.text, "default") == 0))
          break;
        if (p->cur.type == TOK_PUNCT && p->cur.text[0] == '}')
          break;
        int old_pos = p->pos;
        ASTNode_t* s = parse_statement(p);
        if (s) add_child(c, s);
        if (p->pos == old_pos) next_token(p);
      }
      add_child(body, c);
    }
    else if (p->cur.type == TOK_KEYWORD && strcmp(p->cur.text, "default") == 0) {
      next_token(p);
      require_op(p, ":");
      ASTNode_t* d = new_node(AST_DEFAULT);
      while (p->cur.type != TOK_EOF) {
        if (p->cur.type == TOK_KEYWORD &&
            (strcmp(p->cur.text, "case") == 0 || strcmp(p->cur.text, "default") == 0))
          break;
        if (p->cur.type == TOK_PUNCT && p->cur.text[0] == '}')
          break;
        int old_pos = p->pos;
        ASTNode_t* s = parse_statement(p);
        if (s) add_child(d, s);
        if (p->pos == old_pos) next_token(p);
      }
      add_child(body, d);
    }
    else {
      next_token(p);
    }
  }
  require_punct(p, "}");
  return node;
}

static ASTNode_t* parse_for(CDM_Parser_t* p) {
  match_kw(p, "for");
  require_punct(p, "(");
  ASTNode_t* node = new_node(AST_FOR);
  scope_enter();
  if (p->cur.type == TOK_PUNCT && p->cur.text[0] == ';') next_token(p);
  else {
    if (is_type_keyword(p) || (p->cur.type == TOK_IDENT && is_user_type(p->cur.text))) {
      char type_name[64] = { 0 };
      CDM_ValueType_t dt = CDM_TYPE_INT;
      int sid = -1, is_auto = 0;
      if (p->cur.type == TOK_KEYWORD && strcmp(p->cur.text, "auto") == 0) {
        is_auto = 1; next_token(p);
      } else if (is_type_keyword(p)) {
        dt = type_from_keyword(p->cur.text);
        next_token(p);
        while (is_type_keyword(p)) next_token(p);
      } else {
        strncpy(type_name, p->cur.text, 63);
        sid = find_struct_def(type_name);
        dt = CDM_TYPE_STRUCT;
        next_token(p);
      }
      while (p->cur.type == TOK_OP && strcmp(p->cur.text, "*") == 0) { dt = CDM_TYPE_PTR; next_token(p); }
      if (p->cur.type == TOK_IDENT) {
        char varname[64];
        strncpy(varname, p->cur.text, 63); varname[63] = '\0';
        next_token(p);
        if (p->cur.type == TOK_OP && strcmp(p->cur.text, ":") == 0) {
          next_token(p);
          ASTNode_t* arr = parse_expression(p);
          require_punct(p, ")");
          node->type = AST_RANGE_FOR;
          strncpy(node->name, varname, 63);
          node->decl_type = dt;
          node->struct_id = sid;
          node->is_auto = is_auto;
          node->left = arr;
          node->body = parse_statement(p);
          return node;
        }
        ASTNode_t* v = new_node(AST_VAR_DECL);
        strncpy(v->name, varname, 63);
        v->decl_type = dt; v->struct_id = sid; v->is_auto = is_auto;
        if (match_op(p, "=")) v->left = parse_expression(p);
        node->init = v;
      }
    } else {
      ASTNode_t* e = parse_expression(p);
      ASTNode_t* stmt = new_node(AST_EXPR_STMT);
      stmt->left = e;
      node->init = stmt;
    }
    require_punct(p, ";");
  }
  if (!(p->cur.type == TOK_PUNCT && p->cur.text[0] == ';')) node->left = parse_expression(p);
  require_punct(p, ";");
  if (!(p->cur.type == TOK_PUNCT && p->cur.text[0] == ')')) node->update = parse_expression(p);
  require_punct(p, ")");
  node->body = parse_statement(p);
  return node;
}

static ASTNode_t* parse_return(CDM_Parser_t* p) {
  match_kw(p, "return");
  ASTNode_t* node = new_node(AST_RETURN);
  if (!(p->cur.type == TOK_PUNCT && p->cur.text[0] == ';'))
    node->left = parse_expression(p);
  require_punct(p, ";");
  return node;
}
static ASTNode_t* parse_delete(CDM_Parser_t* p) {
  match_kw(p, "delete");
  ASTNode_t* node;
  if (p->cur.type == TOK_PUNCT && p->cur.text[0] == '[') {
    next_token(p);
    require_punct(p, "]");
    node = new_node(AST_DELETE_ARRAY);
  } else {
    node = new_node(AST_DELETE);
  }
  node->left = parse_expression(p);
  require_punct(p, ";");
  return node;
}
static ASTNode_t* parse_block(CDM_Parser_t* p) {
  if (!match_punct(p, "{")) return NULL;
  ASTNode_t* block = new_node(AST_BLOCK);
  while (p->cur.type != TOK_EOF &&
         !(p->cur.type == TOK_PUNCT && p->cur.text[0] == '}')) {
    int old_pos = p->pos;
    ASTNode_t* s = parse_statement(p);
    if (s) add_child(block, s);
    if (p->pos == old_pos) next_token(p);
    if (g_error_count >= CDM_MAX_ERRORS) break;
  }
  require_punct(p, "}");
  return block;
}

static ASTNode_t* parse_struct_def(CDM_Parser_t* p) {
  match_kw(p, "struct");
  char sname[32];
  if (!require_identifier(p, sname, sizeof(sname))) return NULL;
  if (find_struct_def(sname) >= 0) {
    report_error(p, "struct '%s' already defined", sname);
    g_panic_mode = 1;
    return NULL;
  }
  if (g_struct_def_count >= CDM_MAX_STRUCT_TYPES) {
    report_error(p, "too many structs (max %d)", CDM_MAX_STRUCT_TYPES);
    g_panic_mode = 1;
    return NULL;
  }
  int sid = g_struct_def_count++;
  CDM_StructDef_t* sd = &g_struct_defs[sid];
  strncpy(sd->name, sname, 31); sd->name[31] = '\0';
  sd->field_count = 0;
  require_punct(p, "{");
  while (p->cur.type != TOK_EOF &&
         !(p->cur.type == TOK_PUNCT && p->cur.text[0] == '}')) {
    if (sd->field_count >= CDM_MAX_FIELDS_PER_STRUCT) {
      report_error(p, "too many fields in struct (max %d)", CDM_MAX_FIELDS_PER_STRUCT);
      g_panic_mode = 1;
      break;
    }
    if (!is_type_keyword(p)) {
      report_error(p, "expected type in struct, got '%s'", p->cur.text);
      g_panic_mode = 1;
      recover_statement(p);
      continue;
    }
    CDM_ValueType_t ft = type_from_keyword(p->cur.text);
    next_token(p);
    while (is_type_keyword(p)) next_token(p);
    while (p->cur.type == TOK_OP && strcmp(p->cur.text, "*") == 0) { ft = CDM_TYPE_PTR; next_token(p); }
    char fname[32];
    if (!require_identifier(p, fname, sizeof(fname))) {
      recover_statement(p);
      break;
    }
    CDM_FieldDef_t* f = &sd->fields[sd->field_count];
    strncpy(f->name, fname, 31); f->name[31] = '\0';
    f->type = ft; f->struct_id = -1; f->array_len = 0;
    if (p->cur.type == TOK_PUNCT && p->cur.text[0] == '[') {
      next_token(p);
      if (p->cur.type == TOK_NUMBER) {
        f->array_len = atoi(p->cur.text);
        next_token(p);
      } else {
        report_error(p, "expected constant array size, got '%s'", p->cur.text);
        g_panic_mode = 1;
      }
      require_punct(p, "]");
    }
    sd->field_count++;
    require_punct(p, ";");
  }
  require_punct(p, "}");
  require_punct(p, ";");
  ASTNode_t* node = new_node(AST_STRUCT_DEF);
  strncpy(node->name, sname, 63);
  node->struct_id = sid;
  return node;
}

static ASTNode_t* parse_enum(CDM_Parser_t* p) {
  match_kw(p, "enum");
  if (p->cur.type == TOK_IDENT) next_token(p);

  require_punct(p, "{");

  int64_t next_val = 0;
  while (p->cur.type != TOK_EOF &&
         !(p->cur.type == TOK_PUNCT && p->cur.text[0] == '}')) {
    char name[64];
    if (!require_identifier(p, name, sizeof(name))) {
      recover_statement(p);
      break;
    }

    if (match_op(p, "=")) {
      ASTNode_t* ve = parse_expression(p);
      next_val = eval_const(ve);
      CDM_FreeAST(ve);
    }

    if (g_enum_count < MAX_ENUM_ENTRIES) {
      strncpy(g_enums[g_enum_count].name, name, 63);
      g_enums[g_enum_count].name[63] = '\0';
      g_enums[g_enum_count].value = next_val;
      g_enum_count++;
    } else {
      report_error(p, "too many enum entries");
      g_panic_mode = 1;
    }
    next_val++;

    if (!match_punct(p, ",")) break;
  }
  require_punct(p, "}");
  require_punct(p, ";");

  return new_node(AST_ENUM_DECL);
}

static ASTNode_t* parse_declaration(CDM_Parser_t* p, int consume_semi) {
  if (!is_type_keyword(p) && !(p->cur.type == TOK_IDENT && is_user_type(p->cur.text))) return NULL;

  CDM_ValueType_t dt = CDM_TYPE_INT;
  int sid = -1, is_auto = 0;
  if (p->cur.type == TOK_KEYWORD && strcmp(p->cur.text, "auto") == 0) {
    is_auto = 1; next_token(p);
  } else if (is_type_keyword(p)) {
    dt = type_from_keyword(p->cur.text);
    next_token(p);
    while (is_type_keyword(p)) next_token(p);
  } else {
    sid = find_struct_def(p->cur.text);
    dt = CDM_TYPE_STRUCT;
    next_token(p);
  }
  int is_ptr = 0;
  while (p->cur.type == TOK_OP && strcmp(p->cur.text, "*") == 0) {
    is_ptr = 1;
    next_token(p);
    while (p->cur.type == TOK_OP && strcmp(p->cur.text, "*") == 0) next_token(p);
  }
  if (is_ptr) dt = CDM_TYPE_PTR;

  char name[64];
  if (!require_identifier(p, name, sizeof(name))) return NULL;

  if (p->cur.type == TOK_PUNCT && p->cur.text[0] == '(') {
    ASTNode_t* func = new_node(AST_FUNC_DECL);
    strncpy(func->name, name, 63);
    func->decl_type = dt;
    func->struct_id = sid;
    next_token(p);
    int argc = 0;
    ASTNode_t* params[32];
    while (!(p->cur.type == TOK_PUNCT && p->cur.text[0] == ')')) {
      CDM_ValueType_t pt = CDM_TYPE_INT;
      int psid = -1;
      if (is_type_keyword(p)) {
        pt = type_from_keyword(p->cur.text);
        next_token(p);
        while (is_type_keyword(p)) next_token(p);
      } else if (p->cur.type == TOK_IDENT && is_user_type(p->cur.text)) {
        psid = find_struct_def(p->cur.text);
        pt = CDM_TYPE_STRUCT;
        next_token(p);
      }
      while (p->cur.type == TOK_OP && strcmp(p->cur.text, "*") == 0) { pt = CDM_TYPE_PTR; next_token(p); }
      if (p->cur.type == TOK_IDENT) {
        ASTNode_t* param = new_node(AST_PARAM);
        strncpy(param->name, p->cur.text, 63);
        param->decl_type = pt;
        param->struct_id = psid;
        next_token(p);
        while (p->cur.type == TOK_PUNCT && p->cur.text[0] == '[') {
          param->array_dim++;
          next_token(p);
          if (p->cur.type == TOK_NUMBER) { param->array_len = atoi(p->cur.text); next_token(p); }
          while (!(p->cur.type == TOK_PUNCT && p->cur.text[0] == ']')) next_token(p);
          next_token(p);
        }
        if (argc < 32) params[argc++] = param;
      } else {
        report_error(p, "expected parameter name, got '%s'", p->cur.text);
        g_panic_mode = 1;
        break;
      }
      if (!match_punct(p, ",")) break;
    }
    require_punct(p, ")");
    if (argc > 0) {
      func->args = (ASTNode_t**)calloc(argc, sizeof(ASTNode_t*));
      for (int i = 0; i < argc; i++) func->args[i] = params[i];
      func->arg_count = argc;
    }
    func->body = parse_block(p);
    return func;
  }

  if (p->cur.type == TOK_PUNCT && p->cur.text[0] == '[') {
    ASTNode_t* arr = new_node(AST_ARRAY_DECL);
    strncpy(arr->name, name, 63);
    arr->decl_type = dt;
    arr->struct_id = sid;
    arr->array_dim = 0;
    arr->array_len = 1;
    while (p->cur.type == TOK_PUNCT && p->cur.text[0] == '[') {
      next_token(p);
      if (p->cur.type != TOK_NUMBER) {
        report_error(p, "array size must be constant, got '%s'", p->cur.text);
        g_panic_mode = 1;
        while (p->cur.type != TOK_EOF &&
               !(p->cur.type == TOK_PUNCT && p->cur.text[0] == ']'))
          next_token(p);
        match_punct(p, "]");
        break;
      }
      int sz = atoi(p->cur.text);
      if (arr->array_dim < 4) arr->array_sizes[arr->array_dim] = sz;
      arr->array_len *= sz;
      arr->array_dim++;
      next_token(p);
      require_punct(p, "]");
    }
    if (match_op(p, "=")) {
      if (p->cur.type == TOK_PUNCT && p->cur.text[0] == '{') {
        next_token(p);
        ASTNode_t* init_list = new_node(AST_ARRAY_INIT);
        ASTNode_t* items[64];
        int n = 0;
        if (!(p->cur.type == TOK_PUNCT && p->cur.text[0] == '}')) {
          while (n < 64) {
            items[n++] = parse_expression(p);
            if (!match_punct(p, ",")) break;
          }
        }
        require_punct(p, "}");
        init_list->elems = (ASTNode_t**)calloc(n, sizeof(ASTNode_t*));
        for (int i = 0; i < n; i++) init_list->elems[i] = items[i];
        init_list->elem_count = n;
        arr->init = init_list;
      } else {
        report_error(p, "expected '{' for array initializer, got '%s'", p->cur.text);
        g_panic_mode = 1;
      }
    }
    if (consume_semi) require_punct(p, ";");
    return arr;
  }

  ASTNode_t* v = new_node(AST_VAR_DECL);
  strncpy(v->name, name, 63);
  v->is_ptr = is_ptr;
  v->is_auto = is_auto;
  v->decl_type = dt;
  v->struct_id = sid;
  if (match_op(p, "=")) v->left = parse_expression(p);
  if (consume_semi) require_punct(p, ";");
  return v;
}

static ASTNode_t* parse_statement(CDM_Parser_t* p) {
  if (g_panic_mode) {
    recover_statement(p);
    return NULL;
  }

  if (p->cur.type == TOK_KEYWORD && strcmp(p->cur.text, "struct") == 0) return parse_struct_def(p);
  if (p->cur.type == TOK_KEYWORD && strcmp(p->cur.text, "enum") == 0) return parse_enum(p);
  if (p->cur.type == TOK_KEYWORD && strcmp(p->cur.text, "return") == 0) return parse_return(p);
  if (p->cur.type == TOK_KEYWORD && strcmp(p->cur.text, "if") == 0) return parse_if(p);
  if (p->cur.type == TOK_KEYWORD && strcmp(p->cur.text, "while") == 0) return parse_while(p);
  if (p->cur.type == TOK_KEYWORD && strcmp(p->cur.text, "for") == 0) return parse_for(p);
  if (p->cur.type == TOK_KEYWORD && strcmp(p->cur.text, "do") == 0) return parse_do_while(p);
  if (p->cur.type == TOK_KEYWORD && strcmp(p->cur.text, "switch") == 0) return parse_switch(p);
  if (p->cur.type == TOK_KEYWORD && strcmp(p->cur.text, "delete") == 0) return parse_delete(p);

  if (p->cur.type == TOK_KEYWORD &&
      (strcmp(p->cur.text, "break") == 0 || strcmp(p->cur.text, "continue") == 0)) {
    ASTNode_t* n = new_node(strcmp(p->cur.text, "break") == 0 ? AST_BREAK : AST_CONTINUE);
    next_token(p);
    require_punct(p, ";");
    return n;
  }

  if (is_type_keyword(p) || (p->cur.type == TOK_IDENT && is_user_type(p->cur.text)))
    return parse_declaration(p, 1);
  if (p->cur.type == TOK_PUNCT && p->cur.text[0] == '{') return parse_block(p);

  if (p->cur.type == TOK_PUNCT && p->cur.text[0] == ';') {
    next_token(p);
    return NULL;
  }

  ASTNode_t* e = parse_expression(p);
  require_punct(p, ";");
  ASTNode_t* stmt = new_node(AST_EXPR_STMT);
  stmt->left = e;
  return stmt;
}

ASTNode_t* CDM_Parse(const char* source) {
  if (!cdm_ensure_alloc()) return NULL;
  CDM_ClearErrors();
  g_struct_def_count = 0;
  g_enum_count = 0;
  g_parse_line = 1;
  g_panic_mode = 0;
  CDM_Parser_t p;
  p.src = source;
  p.pos = 0;
  p.len = (int)strlen(source);
  p.line = 1;
  ASTNode_t* program = new_node(AST_PROGRAM);
  next_token(&p);
  int tick = 0;
  while (p.cur.type != TOK_EOF) {
    if ((++tick & 0x3F) == 0) {
      printf("[Parse] tick=%d pos=%d/%d line=%d tok='%s'\n",
             tick, p.pos, p.len, p.line, p.cur.text);
    }
    if (g_error_count >= CDM_MAX_ERRORS) break;

    if (g_panic_mode) {
      recover_statement(&p);
      continue;
    }

    int old_pos = p.pos;
    ASTNode_t* decl = parse_declaration(&p, 1);
    if (!decl) decl = parse_statement(&p);
    if (decl) add_child(program, decl);
    else if (p.pos == old_pos) {
      report_error(&p, "unexpected token '%s' at top level", p.cur.text);
      g_panic_mode = 1;
      next_token(&p);
    }
  }
  printf("[Parse] done pos=%d, %d error(s)\n", p.pos, g_error_count);
  return program;
}

void CDM_FreeAST(ASTNode_t* node) {
  if (!node) return;
  ASTNode_t* save_first_child = node->first_child; node->first_child = NULL;
  ASTNode_t* save_left = node->left;               node->left = NULL;
  ASTNode_t* save_right = node->right;             node->right = NULL;
  ASTNode_t* save_body = node->body;               node->body = NULL;
  ASTNode_t* save_init = node->init;               node->init = NULL;
  ASTNode_t* save_update = node->update;           node->update = NULL;
  ASTNode_t* save_cond = node->cond;               node->cond = NULL;
  ASTNode_t* save_step = node->step;               node->step = NULL;
  ASTNode_t** save_args = node->args;              node->args = NULL;
  int save_args_n = node->arg_count;               node->arg_count = 0;
  ASTNode_t** save_elems = node->elems;            node->elems = NULL;
  int save_elems_n = node->elem_count;             node->elem_count = 0;
  ASTNode_t* save_next = node->next_sibling;       node->next_sibling = NULL;
  (void)save_next;

  ASTNode_t* c = save_first_child;
  while (c) {
    ASTNode_t* next = c->next_sibling;
    CDM_FreeAST(c);
    c = next;
  }
  if (save_args) {
    for (int i = 0; i < save_args_n; i++) CDM_FreeAST(save_args[i]);
    free(save_args);
  }
  if (save_elems) {
    for (int i = 0; i < save_elems_n; i++) CDM_FreeAST(save_elems[i]);
    free(save_elems);
  }
  if (save_left && save_left != save_first_child) CDM_FreeAST(save_left);
  if (save_right && save_right != save_first_child) CDM_FreeAST(save_right);
  if (save_body && save_body != save_first_child) CDM_FreeAST(save_body);
  if (save_init && save_init != save_first_child) CDM_FreeAST(save_init);
  if (save_update && save_update != save_first_child) CDM_FreeAST(save_update);
  if (save_cond && save_cond != save_first_child) CDM_FreeAST(save_cond);
  if (save_step && save_step != save_first_child) CDM_FreeAST(save_step);
  free(node);
}

static int g_returning = 0;
static CDM_Value_t g_return_value;

static CDM_Value_t eval_primary(ASTNode_t* node);
static int exec_statement(ASTNode_t* node);
static int resolve_any_lvalue(ASTNode_t* node, CDM_Value_t* out_slot);

static int is_cout_chain(ASTNode_t* node) {
  if (!node) return 0;
  if (node->type == AST_MEMBER && strcmp(node->name, "std") == 0 &&
      node->first_child && strcmp(node->first_child->name, "cout") == 0) return 1;
  if (node->type == AST_BINARY && strcmp(node->name, "<<") == 0) return is_cout_chain(node->left);
  return 0;
}
static int is_cin_chain(ASTNode_t* node) {
  if (!node) return 0;
  if (node->type == AST_MEMBER && strcmp(node->name, "std") == 0 &&
      node->first_child && strcmp(node->first_child->name, "cin") == 0) return 1;
  if (node->type == AST_BINARY && strcmp(node->name, ">>") == 0) return is_cin_chain(node->left);
  return 0;
}

static void output_value(ASTNode_t* node) {
  if (!node) return;
  if (node->type == AST_STRING) {
    CDM_HostFunc_t fn = find_host_func("__print_str");
    if (fn) { const char* args[1] = { node->name }; fn(1, args); }
    return;
  }
  CDM_Value_t v = CDM_Deref(eval_primary(node));
  if (v.type == CDM_TYPE_STRING) {
    CDM_HostFunc_t fn = find_host_func("__print_str");
    if (fn) { const char* args[1] = { val_to_str_raw(v) }; fn(1, args); }
  } else if (v.type == CDM_TYPE_FLOAT) {
    CDM_HostFunc_t fn = find_host_func("__print_float");
    if (fn) {
      char buf[32]; snprintf(buf, sizeof(buf), "%g", v.v.f);
      const char* args[1] = { buf }; fn(1, args);
    }
  } else if (v.type == CDM_TYPE_ARRAY || v.type == CDM_TYPE_STRUCT) {
    CDM_HostFunc_t fn = find_host_func("__print_str");
    if (fn) { const char* args[1] = { "<object>" }; fn(1, args); }
  } else {
    CDM_HostFunc_t fn = find_host_func("__print_int");
    if (fn) {
      char buf[32]; snprintf(buf, sizeof(buf), "%lld", (long long)val_to_int_raw(v));
      const char* args[1] = { buf }; fn(1, args);
    }
  }
  if (v.owned) value_release(v);
}

static void input_value(ASTNode_t* node) {
  if (!node || node->type != AST_IDENTIFIER) return;
  Symbol_t* s = find_symbol(node->name);
  if (!s) s = create_symbol(node->name);
  if (!s) return;
  if (s->value.type == CDM_TYPE_FLOAT) {
    CDM_HostFunc_t fn = find_host_func("__read_float");
    if (fn) {
      int32_t bits = (int32_t)fn(0, NULL);
      float f; memcpy(&f, &bits, sizeof(f));
      s->value.v.f = (double)f;
    }
  } else if (s->value.type == CDM_TYPE_STRING) {
    CDM_HostFunc_t fn = find_host_func("__read_str");
    if (fn) {
      const char* s2 = (const char*)fn(0, NULL);
      if (s2) { value_release(s->value); s->value = CDM_MakeString(s2); }
    }
  } else {
    CDM_HostFunc_t fn = find_host_func("__read_int");
    if (fn) s->value.v.i = fn(0, NULL);
  }
}

static ASTNode_t* collect_indices(ASTNode_t* node, ASTNode_t** idxs, int* dim) {
  if (!node) return NULL;
  if (node->type == AST_ARRAY_SUBSCRIPT) {
    ASTNode_t* base = collect_indices(node->left, idxs, dim);
    if (*dim < 8) idxs[(*dim)++] = node->right;
    return base;
  }
  return node;
}
static int flatten_index(CDM_Value_t arr, ASTNode_t** idxs, int dim) {
  if (dim <= 0) return 0;
  int n = dim < arr.array_dim ? dim : arr.array_dim;
  int offset = 0;
  for (int i = 0; i < n; i++) {
    CDM_Value_t iv = CDM_Deref(eval_primary(idxs[i]));
    int idx = (int)val_to_int_raw(iv);
    if (iv.owned) value_release(iv);
    if (idx < 0 || idx >= arr.array_sizes[i]) {
      report_error(NULL, "array index [%d]=%d out of range [0,%d)", i, idx, arr.array_sizes[i]);
      return -1;
    }
    int mult = 1;
    for (int j = i + 1; j < arr.array_dim; j++) mult *= arr.array_sizes[j];
    offset += idx * mult;
  }
  return offset;
}
static int resolve_array_lvalue(ASTNode_t* node, CDM_Value_t* out_arr, int* out_idx) {
  if (!node || node->type != AST_ARRAY_SUBSCRIPT) return 0;
  ASTNode_t* idxs[8];
  int dim = 0;
  ASTNode_t* base = collect_indices(node, idxs, &dim);
  if (!base || base->type != AST_IDENTIFIER) return 0;
  Symbol_t* s = find_symbol(base->name);
  if (!s || s->value.type != CDM_TYPE_ARRAY) return 0;
  int off = flatten_index(s->value, idxs, dim);
  if (off < 0) return 0;
  *out_arr = s->value;
  *out_idx = off;
  return 1;
}
static int resolve_member_lvalue(ASTNode_t* node, CDM_Value_t* out_struct, int* out_field) {
  if (!node || node->type != AST_MEMBER_ACCESS) return 0;
  CDM_Value_t obj;
  if (node->left->type == AST_IDENTIFIER) {
    Symbol_t* s = find_symbol(node->left->name);
    if (!s) return 0;
    obj = CDM_Deref(s->value);
  } else if (node->left->type == AST_MEMBER_ACCESS) {
    CDM_Value_t inner; int f;
    if (!resolve_member_lvalue(node->left, &inner, &f)) return 0;
    inner = CDM_Deref(inner);
    if (inner.type != CDM_TYPE_STRUCT || !inner.v.p) return 0;
    obj = ((CDM_Value_t*)inner.v.p)[f];
  } else if (node->left->type == AST_ARRAY_SUBSCRIPT) {
    CDM_Value_t arr; int idx;
    if (!resolve_array_lvalue(node->left, &arr, &idx)) return 0;
    if (arr.type != CDM_TYPE_ARRAY || !arr.v.p) return 0;
    obj = ((CDM_Value_t*)arr.v.p)[idx];
  } else {
    obj = CDM_Deref(eval_primary(node->left));
  }
  obj = CDM_Deref(obj);
  if (obj.type != CDM_TYPE_STRUCT || obj.struct_id < 0) return 0;
  int fidx = find_field_index(obj.struct_id, node->name);
  if (fidx < 0) return 0;
  *out_struct = obj;
  *out_field = fidx;
  return 1;
}
static int resolve_any_lvalue(ASTNode_t* node, CDM_Value_t* out_slot) {
  if (!node) return 0;
  if (node->type == AST_IDENTIFIER) {
    Symbol_t* s = find_symbol(node->name);
    if (!s) s = create_symbol(node->name);
    if (!s) return 0;
    *out_slot = make_ref(&s->value);
    return 1;
  }
  if (node->type == AST_MEMBER_ACCESS) {
    CDM_Value_t obj; int f;
    if (resolve_member_lvalue(node, &obj, &f)) {
      *out_slot = make_ref(&((CDM_Value_t*)obj.v.p)[f]);
      return 1;
    }
  }
  if (node->type == AST_ARRAY_SUBSCRIPT) {
    CDM_Value_t arr; int idx;
    if (resolve_array_lvalue(node, &arr, &idx)) {
      *out_slot = make_ref(&((CDM_Value_t*)arr.v.p)[idx]);
      return 1;
    }
  }
  return 0;
}

static CDM_Value_t write_lvalue(ASTNode_t* node, CDM_Value_t r) {
  CDM_Value_t none; init_common(&none); none.type = CDM_TYPE_INT;

  CDM_Value_t slot;
  if (!resolve_any_lvalue(node, &slot)) {
    if (r.owned) value_release(r);
    return none;
  }
  CDM_Value_t* p = (CDM_Value_t*)slot.v.p;
  if (!p) {
    if (r.owned) value_release(r);
    return none;
  }

  if (p->type == CDM_TYPE_STRING && r.type == CDM_TYPE_STRING) {
    if (p->owned && p->v.p) free(p->v.p);
    const char* s = r.v.p ? (const char*)r.v.p : "";
    size_t n = strlen(s);
    char* buf = (char*)malloc(n + 1);
    if (buf) { memcpy(buf, s, n); buf[n] = '\0'; }
    CDM_Value_t nv; init_common(&nv);
    nv.type = CDM_TYPE_STRING; nv.owned = 1; nv.v.p = buf;
    *p = nv;
    if (r.owned) value_release(r);
    CDM_Value_t ret = *p; ret.owned = 0; return ret;
  }
  if (p->type == CDM_TYPE_FLOAT && r.type != CDM_TYPE_FLOAT) {
    double v = val_to_double_raw(r);
    value_release(*p);
    *p = make_float(v);
    if (r.owned) value_release(r);
    CDM_Value_t ret = *p; ret.owned = 0; return ret;
  }
  if (p->type == CDM_TYPE_INT && r.type == CDM_TYPE_FLOAT) {
    value_release(*p);
    *p = make_int((int64_t)r.v.f);
    if (r.owned) value_release(r);
    CDM_Value_t ret = *p; ret.owned = 0; return ret;
  }
  if (p->type == CDM_TYPE_STRUCT && r.type == CDM_TYPE_STRUCT &&
      p->struct_id == r.struct_id && p->v.p && r.v.p) {
    int nf = g_struct_defs[p->struct_id].field_count;
    memcpy(p->v.p, r.v.p, nf * sizeof(CDM_Value_t));
    if (r.owned) value_release(r);
    CDM_Value_t ret = *p; ret.owned = 0; return ret;
  }
  value_release(*p);
  *p = r;
  CDM_Value_t ret = *p; ret.owned = 0; return ret;
}

static CDM_Value_t call_user_func(ASTNode_t* func, ASTNode_t* call) {
  int saved_returning = g_returning;
  CDM_Value_t saved_rv = g_return_value;
  g_returning = 0;
  g_return_value = make_int(0);
  scope_enter();
  int n = func->arg_count < call->arg_count ? func->arg_count : call->arg_count;
  for (int i = 0; i < n; i++) {
    CDM_Value_t v = CDM_Deref(eval_primary(call->args[i]));
    Symbol_t* s = find_symbol(func->args[i]->name);
    if (!s) s = create_symbol(func->args[i]->name);
    if (s) { value_release(s->value); s->value = v; }
  }
  if (func->body) exec_statement(func->body);
  CDM_Value_t result = g_return_value;
  scope_exit();
  g_returning = saved_returning;
  g_return_value = saved_rv;
  return result;
}

static void build_host_args(ASTNode_t* node, CDM_Value_t* out, int* out_n) {
  int n = node->arg_count < 32 ? node->arg_count : 32;
  for (int i = 0; i < n; i++) {
    ASTNode_t* arg = node->args[i];
    CDM_Value_t slot;
    if ((arg->type == AST_IDENTIFIER || arg->type == AST_MEMBER_ACCESS || arg->type == AST_ARRAY_SUBSCRIPT)
        && resolve_any_lvalue(arg, &slot)) {
      CDM_Value_t* p = (CDM_Value_t*)slot.v.p;
      if (p) { out[i] = make_ref(p); continue; }
    }
    out[i] = eval_primary(arg);
  }
  *out_n = n;
}

static CDM_Value_t eval_method_call(ASTNode_t* node) {
  CDM_Value_t obj = CDM_Deref(eval_primary(node->left));
  const char* type_prefix = NULL;
  if (obj.type == CDM_TYPE_STRING) type_prefix = "std::string";
  else if (obj.type == CDM_TYPE_STRUCT && obj.struct_id >= 0)
    type_prefix = g_struct_defs[obj.struct_id].name;
  else {
    if (obj.owned) value_release(obj);
    return make_int(0);
  }

  char fname[128];
  snprintf(fname, sizeof(fname), "%s.%s", type_prefix, node->name);

  CDM_HostFuncEx_t ex = find_host_ex(fname);
  if (!ex) {
    report_error(NULL, "no method '%s'", fname);
    if (obj.owned) value_release(obj);
    return make_int(0);
  }
  CDM_Value_t args[32];
  args[0] = obj;
  int n = node->arg_count < 31 ? node->arg_count : 31;
  for (int i = 0; i < n; i++) args[i + 1] = CDM_Deref(eval_primary(node->args[i]));

  CDM_Value_t result = ex(n + 1, args);

  if (obj.owned) value_release(obj);
  for (int i = 0; i < n; i++)
    if (args[i + 1].owned) value_release(args[i + 1]);

  return result;
}

static CDM_Value_t eval_func_call(ASTNode_t* node) {
  ASTNode_t* uf = find_user_func(node->name);
  if (uf) return call_user_func(uf, node);

  CDM_HostFuncEx_t ex = NULL;
  if (node->cached_he_tried) {
    ex = (CDM_HostFuncEx_t)node->cached_host_ex;
  } else {
    ex = find_host_ex(node->name);
    node->cached_host_ex = (void*)ex;
    node->cached_he_tried = 1;
  }
  if (ex) {
    CDM_Value_t args[32];
    int n = 0;
    build_host_args(node, args, &n);
    CDM_Value_t result = ex(n, args);
    for (int i = 0; i < n; i++)
      if (!CDM_IsRef(args[i]) && args[i].owned) value_release(args[i]);
    return result;
  }

  CDM_HostFunc_t fn = NULL;
  if (node->cached_hf_tried) {
    fn = (CDM_HostFunc_t)node->cached_host_func;
  } else {
    fn = find_host_func(node->name);
    node->cached_host_func = (void*)fn;
    node->cached_hf_tried = 1;
  }
  if (!fn) {
    report_error(NULL, "undefined function '%s'", node->name);
    return make_int(0);
  }

  const char* argv[32];
  char* heap_args[32];
  int heap_count = 0;
  for (int i = 0; i < node->arg_count && i < 32; i++) {
    if (node->args[i]->type == AST_STRING) {
      argv[i] = node->args[i]->name;
    } else {
      CDM_Value_t v = CDM_Deref(eval_primary(node->args[i]));
      char* buf = (char*)malloc(64);
      if (v.type == CDM_TYPE_FLOAT) snprintf(buf, 64, "%g", v.v.f);
      else if (v.type == CDM_TYPE_STRING) snprintf(buf, 64, "%s", val_to_str_raw(v));
      else snprintf(buf, 64, "%lld", (long long)val_to_int_raw(v));
      if (v.owned) value_release(v);
      argv[i] = buf;
      heap_args[heap_count++] = buf;
    }
  }
  int ret = fn(node->arg_count, argv);
  for (int i = 0; i < heap_count; i++) free(heap_args[i]);

  if (is_float_func(node->name)) {
    float f; memcpy(&f, &ret, sizeof(f));
    return make_float((double)f);
  }
  return make_int(ret);
}

static CDM_Value_t do_new(ASTNode_t* node) {
  if (node->type == AST_NEW_ARRAY) {
    int n = 1;
    if (node->left) {
      CDM_Value_t nv = CDM_Deref(eval_primary(node->left));
      n = (int)val_to_int_raw(nv);
      if (nv.owned) value_release(nv);
    }
    if (n <= 0) n = 1;
    CDM_Value_t* arr = array_new(n, node->decl_type, node->struct_id);
    if (!arr) return make_ptr(NULL, 0);
    CDM_Value_t v = make_ptr(arr, 1);
    v.from_new = 1;
    return v;
  } else {
    if (node->decl_type == CDM_TYPE_STRUCT && node->struct_id >= 0) {
      int nf = g_struct_defs[node->struct_id].field_count;
      CDM_Value_t* block = (CDM_Value_t*)calloc(nf, sizeof(CDM_Value_t));
      for (int i = 0; i < nf; i++) block[i] = make_int(0);
      return make_ptr(block, 1);
    } else if (node->decl_type == CDM_TYPE_FLOAT) {
      double* p = (double*)malloc(sizeof(double));
      if (!p) return make_ptr(NULL, 0);
      *p = 0;
      return make_ptr(p, 1);
    } else {
      int64_t* p = (int64_t*)malloc(sizeof(int64_t));
      if (!p) return make_ptr(NULL, 0);
      *p = 0;
      return make_ptr(p, 1);
    }
  }
}

static CDM_Value_t eval_assign(ASTNode_t* node, const char* op) {
  if (strcmp(op, "=") == 0) {
    CDM_Value_t r = CDM_Deref(eval_primary(node->right));
    return write_lvalue(node->left, r);
  }

  CDM_Value_t slot;
  if (!resolve_any_lvalue(node->left, &slot)) {
    CDM_Value_t r = CDM_Deref(eval_primary(node->right));
    if (r.owned) value_release(r);
    return make_int(0);
  }
  CDM_Value_t* p = (CDM_Value_t*)slot.v.p;
  if (!p) return make_int(0);

  CDM_Value_t old = *p;
  CDM_Value_t r = CDM_Deref(eval_primary(node->right));

  if (strcmp(op, "+=") == 0 &&
      old.type == CDM_TYPE_STRING && r.type == CDM_TYPE_STRING) {
    const char* a = val_to_str_raw(old);
    const char* b = val_to_str_raw(r);
    size_t la = strlen(a), lb = strlen(b);
    char* buf = (char*)malloc(la + lb + 1);
    CDM_Value_t nv;
    if (buf) {
      memcpy(buf, a, la);
      memcpy(buf + la, b, lb);
      buf[la + lb] = '\0';
      nv = CDM_MakeStringOwned(buf);
    } else {
      nv = make_int(0);
    }
    if (r.owned) value_release(r);
    value_release(*p);
    *p = nv;
    CDM_Value_t ret = *p; ret.owned = 0;
    return ret;
  }

  if (op[0] == '&' || op[0] == '|' || op[0] == '^' ||
      (op[0] == '<' && op[1] == '<') || (op[0] == '>' && op[1] == '>')) {
    int64_t o = val_to_int_raw(old);
    int64_t x = val_to_int_raw(r);
    int64_t z = o;
    if (strcmp(op, "&=") == 0) z = o & x;
    else if (strcmp(op, "|=") == 0) z = o | x;
    else if (strcmp(op, "^=") == 0) z = o ^ x;
    else if (strcmp(op, "<<=") == 0) z = o << x;
    else if (strcmp(op, ">>=") == 0) z = o >> x;
    if (r.owned) value_release(r);
    value_release(*p);
    *p = make_int(z);
    CDM_Value_t ret = *p; ret.owned = 0;
    return ret;
  }

  double o = val_to_double_raw(old);
  double x = val_to_double_raw(r);
  double z = o;
  if (strcmp(op, "+=") == 0) z = o + x;
  else if (strcmp(op, "-=") == 0) z = o - x;
  else if (strcmp(op, "*=") == 0) z = o * x;
  else if (strcmp(op, "/=") == 0) z = x != 0 ? o / x : 0;
  else if (strcmp(op, "%=") == 0) z = x != 0 ? (double)((int64_t)o % (int64_t)x) : 0;
  if (r.owned) value_release(r);
  value_release(*p);
  *p = (old.type == CDM_TYPE_FLOAT) ? make_float(z) : make_int((int64_t)z);
  CDM_Value_t ret = *p; ret.owned = 0;
  return ret;
}

static CDM_Value_t eval_binary_values(const char* op,
                                      CDM_Value_t l, CDM_Value_t r) {
  if (strcmp(op, "+") == 0 && (val_is_string(l) || val_is_string(r))) {
    const char* ls = val_is_string(l) ? val_to_str_raw(l) : "";
    const char* rs = val_is_string(r) ? val_to_str_raw(r) : "";
    size_t ln = strlen(ls), rn = strlen(rs);
    char* buf = (char*)malloc(ln + rn + 1);
    if (!buf) return make_int(0);
    memcpy(buf, ls, ln);
    memcpy(buf + ln, rs, rn);
    buf[ln + rn] = '\0';
    return CDM_MakeStringOwned(buf);
  }

  if (strcmp(op, "&&") == 0) return make_int(val_to_int_raw(l) && val_to_int_raw(r));
  if (strcmp(op, "||") == 0) return make_int(val_to_int_raw(l) || val_to_int_raw(r));

  if (strcmp(op, "==") == 0) {
    if (val_is_string(l) || val_is_string(r))
      return make_int(strcmp(val_to_str_raw(l), val_to_str_raw(r)) == 0);
    if (val_is_float(l) || val_is_float(r))
      return make_int(val_to_double_raw(l) == val_to_double_raw(r));
    if (val_is_ptr(l) || val_is_ptr(r))
      return make_int(l.v.p == r.v.p);
    return make_int(val_to_int_raw(l) == val_to_int_raw(r));
  }
  if (strcmp(op, "!=") == 0) {
    if (val_is_string(l) || val_is_string(r))
      return make_int(strcmp(val_to_str_raw(l), val_to_str_raw(r)) != 0);
    if (val_is_float(l) || val_is_float(r))
      return make_int(val_to_double_raw(l) != val_to_double_raw(r));
    if (val_is_ptr(l) || val_is_ptr(r))
      return make_int(l.v.p != r.v.p);
    return make_int(val_to_int_raw(l) != val_to_int_raw(r));
  }

  if (strcmp(op, "<") == 0)  return make_int(val_to_double_raw(l) <  val_to_double_raw(r));
  if (strcmp(op, ">") == 0)  return make_int(val_to_double_raw(l) >  val_to_double_raw(r));
  if (strcmp(op, "<=") == 0) return make_int(val_to_double_raw(l) <= val_to_double_raw(r));
  if (strcmp(op, ">=") == 0) return make_int(val_to_double_raw(l) >= val_to_double_raw(r));

  if (val_is_float(l) || val_is_float(r)) {
    double ld = val_to_double_raw(l), rd = val_to_double_raw(r);
    if (strcmp(op, "+") == 0) return make_float(ld + rd);
    if (strcmp(op, "-") == 0) return make_float(ld - rd);
    if (strcmp(op, "*") == 0) return make_float(ld * rd);
    if (strcmp(op, "/") == 0) return make_float(rd != 0 ? ld / rd : 0);
    if (strcmp(op, "%") == 0) return make_float(rd != 0 ? (double)((int64_t)ld % (int64_t)rd) : 0);
  }

  int64_t li = val_to_int_raw(l), ri = val_to_int_raw(r);
  if (strcmp(op, "+") == 0)  return make_int(li + ri);
  if (strcmp(op, "-") == 0)  return make_int(li - ri);
  if (strcmp(op, "*") == 0)  return make_int(li * ri);
  if (strcmp(op, "/") == 0) {
    if (ri == 0) { report_error(NULL, "division by zero"); return make_int(0); }
    return make_int(li / ri);
  }
  if (strcmp(op, "%") == 0) {
    if (ri == 0) { report_error(NULL, "modulo by zero"); return make_int(0); }
    return make_int(li % ri);
  }
  if (strcmp(op, "&") == 0)  return make_int(li & ri);
  if (strcmp(op, "|") == 0)  return make_int(li | ri);
  if (strcmp(op, "^") == 0)  return make_int(li ^ ri);
  if (strcmp(op, "<<") == 0) return make_int(li << ri);
  if (strcmp(op, ">>") == 0) return make_int(li >> ri);
  return make_int(0);
}

static CDM_Value_t eval_binary(ASTNode_t* node) {
  const char* op = node->name;

  if (strcmp(op, "<<") == 0 && is_cout_chain(node)) {
    eval_primary(node->left);
    output_value(node->right);
    return make_int(0);
  }
  if (strcmp(op, ">>") == 0 && is_cin_chain(node)) {
    eval_primary(node->left);
    input_value(node->right);
    return make_int(0);
  }

  if (strcmp(op, "=") == 0 || strcmp(op, "+=") == 0 || strcmp(op, "-=") == 0 ||
      strcmp(op, "*=") == 0 || strcmp(op, "/=") == 0 || strcmp(op, "%=") == 0 ||
      strcmp(op, "&=") == 0 || strcmp(op, "|=") == 0 || strcmp(op, "^=") == 0 ||
      strcmp(op, "<<=") == 0 || strcmp(op, ">>=") == 0) {
    return eval_assign(node, op);
  }

  CDM_Value_t l = CDM_Deref(eval_primary(node->left));
  CDM_Value_t r = CDM_Deref(eval_primary(node->right));

  CDM_Value_t result = eval_binary_values(op, l, r);

  if (l.owned) value_release(l);
  if (r.owned) value_release(r);

  return result;
}

static CDM_Value_t eval_primary(ASTNode_t* node) {
  if (!node) return make_int(0);
  if (node->line) g_current_line = node->line;

  switch (node->type) {
    case AST_NUMBER:
      if (strchr(node->name, '.') || strchr(node->name, 'e') || strchr(node->name, 'E'))
        return make_float(node->fval);
      return make_int(node->ival);
    case AST_STRING:
      return CDM_MakeString(node->name);
    case AST_IDENTIFIER:
      {
        int ei = find_enum(node->name);
        if (ei >= 0) return make_int(g_enums[ei].value);

        Symbol_t* s;
        if (node->cached_sym_tried) {
          s = (Symbol_t*)node->cached_symbol;
        } else {
          s = find_symbol(node->name);
          node->cached_symbol = (void*)s;
          node->cached_sym_tried = 1;
        }
        if (!s) return make_int(0);
        CDM_Value_t v = s->value;
        v.owned = 0;
        v.from_new = 0;
        return v;
      }
    case AST_BINARY: return eval_binary(node);
    case AST_FUNC_CALL: return eval_func_call(node);
    case AST_METHOD_CALL: return eval_method_call(node);
    case AST_NEW:
    case AST_NEW_ARRAY: return do_new(node);
    case AST_MEMBER_ACCESS:
      {
        CDM_Value_t obj; int fidx;
        if (resolve_member_lvalue(node, &obj, &fidx)) {
          CDM_Value_t v = ((CDM_Value_t*)obj.v.p)[fidx];
          v.owned = 0;
          v.from_new = 0;
          return v;
        }
        return make_int(0);
      }
    case AST_ARRAY_SUBSCRIPT:
      {
        CDM_Value_t arr; int idx;
        if (resolve_array_lvalue(node, &arr, &idx)) {
          if (idx < 0 || idx >= arr.array_len) return make_int(0);
          CDM_Value_t v = ((CDM_Value_t*)arr.v.p)[idx];
          v.owned = 0;
          v.from_new = 0;
          return v;
        }
        return make_int(0);
      }
    case AST_TERNARY:
      {
        CDM_Value_t c = CDM_Deref(eval_primary(node->left));
        int cond = val_to_int_raw(c);
        if (c.owned) value_release(c);
        return cond ? eval_primary(node->right) : eval_primary(node->init);
      }
    case AST_UNARY:
      {
        if (strcmp(node->name, "++") == 0 || strcmp(node->name, "--") == 0) {
          CDM_Value_t slot;
          if (resolve_any_lvalue(node->left, &slot)) {
            CDM_Value_t* p = (CDM_Value_t*)slot.v.p;
            if (p) {
              CDM_Value_t old = *p;
              int delta = (node->name[0] == '+') ? 1 : -1;
              if (old.type == CDM_TYPE_FLOAT) p->v.f += delta;
              else p->v.i += delta;
              CDM_Value_t ret = node->is_ptr ? old : *p;
              ret.owned = 0;
              return ret;
            }
          }
          return make_int(0);
        }
        if (strcmp(node->name, "*") == 0) {
          CDM_Value_t ptr = CDM_Deref(eval_primary(node->left));
          if (ptr.type == CDM_TYPE_REF && ptr.v.p) {
            CDM_Value_t v = *(CDM_Value_t*)ptr.v.p;
            v.owned = 0;
            return v;
          }
          if (ptr.type == CDM_TYPE_PTR && ptr.v.p)
            return make_ptr(ptr.v.p, 0);
          return make_int(0);
        }
        if (strcmp(node->name, "&") == 0) {
          if (node->left->type == AST_IDENTIFIER) {
            Symbol_t* s = find_symbol(node->left->name);
            if (!s) s = create_symbol(node->left->name);
            if (s) return make_ref(&s->value);
          }
          return make_ptr(NULL, 0);
        }
        CDM_Value_t v = CDM_Deref(eval_primary(node->left));
        CDM_Value_t result;
        if (strcmp(node->name, "-") == 0) {
          result = val_is_float(v) ? make_float(-v.v.f) : make_int(-v.v.i);
        } else if (strcmp(node->name, "!") == 0) {
          result = make_int(!val_to_int_raw(v));
        } else if (strcmp(node->name, "~") == 0) {
          result = make_int(~val_to_int_raw(v));
        } else {
          result = v;
          result.owned = 0;
        }
        if (v.owned) value_release(v);
        return result;
      }
    case AST_ENUM_DECL:
      return make_int(0);

    case AST_SIZEOF:
      {
        if (node->left && node->left->type == AST_ARRAY_SUBSCRIPT) {
          CDM_Value_t arr; int idx;
          if (resolve_array_lvalue(node->left, &arr, &idx)) {
            int64_t es = 4;
            if (arr.array_elem_type == CDM_TYPE_FLOAT) es = 8;
            else if (arr.array_elem_type == CDM_TYPE_PTR) es = 8;
            return make_int(es);
          }
        }
        if (node->left && node->left->type == AST_IDENTIFIER) {
          Symbol_t* s = find_symbol(node->left->name);
          if (s) return make_int(sizeof_value(s->value));
        }
        CDM_Value_t v = CDM_Deref(eval_primary(node->left));
        int64_t sz = sizeof_value(v);
        if (v.owned) value_release(v);
        return make_int(sz);
      }

    case AST_CAST:
      {
        CDM_Value_t v = CDM_Deref(eval_primary(node->left));
        CDM_Value_t result;
        init_common(&result);
        if (node->decl_type == CDM_TYPE_FLOAT) {
          result = make_float(val_to_double_raw(v));
        } else if (node->decl_type == CDM_TYPE_PTR) {
          void* ptr = (v.type == CDM_TYPE_PTR) ? v.v.p : (void*)(intptr_t)val_to_int_raw(v);
          result = make_ptr(ptr, 0);
        } else {
          result = make_int(val_to_int_raw(v));
        }
        if (v.owned) value_release(v);
        return result;
      }

    case AST_MEMBER: return make_int(0);
  }
  return make_int(0);
}

static int exec_statement(ASTNode_t* node) {
  if (!node || g_returning || g_break_flag || g_continue_flag) return 0;
  if (node->line) g_current_line = node->line;

  switch (node->type) {
    case AST_BLOCK:
      {
        scope_enter();
        ASTNode_t* c = node->first_child;
        while (c && !g_returning && !g_break_flag && !g_continue_flag) {
          exec_statement(c);
          c = c->next_sibling;
        }
        scope_exit();
        return 0;
      }
    case AST_RETURN:
      g_returning = 1;
      g_return_value = node->left ? eval_primary(node->left) : make_int(0);
      return 0;

    case AST_BREAK:    g_break_flag = 1;    return 0;
    case AST_CONTINUE: g_continue_flag = 1; return 0;

    case AST_VAR_DECL:
      {
        Symbol_t* s;
        if (node->cached_sym_tried) {
          s = (Symbol_t*)node->cached_symbol;
        } else {
          s = find_symbol(node->name);
          if (!s) s = create_symbol(node->name);
          node->cached_symbol = (void*)s;
          node->cached_sym_tried = 1;
        }
        if (!s) return 0;

        if (node->decl_type == CDM_TYPE_STRUCT && node->struct_id >= 0 && !node->is_auto) {
          int nf = g_struct_defs[node->struct_id].field_count;
          CDM_Value_t* block = (CDM_Value_t*)calloc(nf, sizeof(CDM_Value_t));
          for (int i = 0; i < nf; i++) block[i] = make_int(0);
          value_release(s->value);
          s->value = make_struct(block, node->struct_id, 1);
          if (node->left) {
            CDM_Value_t r = CDM_Deref(eval_primary(node->left));
            if (r.type == CDM_TYPE_STRUCT && r.v.p && r.struct_id == node->struct_id)
              memcpy(block, r.v.p, nf * sizeof(CDM_Value_t));
            if (r.owned) value_release(r);
          }
        } else {
          if (node->left) {
            CDM_Value_t v = CDM_Deref(eval_primary(node->left));
            if (node->is_auto) {
              value_release(s->value);
              s->value = v;
              s->value.owned = 0;
              if (v.type == CDM_TYPE_STRING) {
                s->value = CDM_MakeString(val_to_str_raw(v));
                if (v.owned) value_release(v);
              }
            } else if (v.type == CDM_TYPE_STRING && node->decl_type == CDM_TYPE_STRING) {
              value_release(s->value); s->value = CDM_MakeString(val_to_str_raw(v));
              if (v.owned) value_release(v);
            } else if (node->decl_type == CDM_TYPE_FLOAT && v.type != CDM_TYPE_FLOAT) {
              value_release(s->value); s->value = make_float((double)val_to_double_raw(v));
              if (v.owned) value_release(v);
            } else if (node->decl_type == CDM_TYPE_INT && v.type == CDM_TYPE_FLOAT) {
              value_release(s->value); s->value = make_int((int64_t)v.v.f);
              if (v.owned) value_release(v);
            } else {
              value_release(s->value);
              s->value = v;
            }
          } else {
            value_release(s->value);
            if (node->decl_type == CDM_TYPE_FLOAT) s->value = make_float(0.0);
            else if (node->decl_type == CDM_TYPE_PTR) s->value = make_ptr(NULL, 0);
            else if (node->decl_type == CDM_TYPE_STRING) s->value = CDM_MakeString("");
            else s->value = make_int(0);
          }
        }
        return 0;
      }
    case AST_ARRAY_DECL:
      {
        Symbol_t* s;
        if (node->cached_sym_tried) {
          s = (Symbol_t*)node->cached_symbol;
        } else {
          s = find_symbol(node->name);
          if (!s) s = create_symbol(node->name);
          node->cached_symbol = (void*)s;
          node->cached_sym_tried = 1;
        }
        if (!s) return 0;
        int n = node->array_len > 0 ? node->array_len : 1;
        CDM_Value_t* block = (CDM_Value_t*)calloc(n, sizeof(CDM_Value_t));
        if (node->decl_type == CDM_TYPE_STRUCT && node->struct_id >= 0) {
          int nf = g_struct_defs[node->struct_id].field_count;
          for (int i = 0; i < n; i++) {
            CDM_Value_t* fblock = (CDM_Value_t*)calloc(nf, sizeof(CDM_Value_t));
            for (int j = 0; j < nf; j++) fblock[j] = make_int(0);
            block[i] = make_struct(fblock, node->struct_id, 1);
          }
        } else if (node->decl_type == CDM_TYPE_STRING) {
          for (int i = 0; i < n; i++) block[i] = CDM_MakeString("");
        } else {
          for (int i = 0; i < n; i++) block[i] = make_int(0);
        }
        value_release(s->value);
        s->value = make_array(block, n, node->array_dim,
                              node->array_sizes, node->decl_type, node->struct_id, 1);
        if (node->init && node->init->type == AST_ARRAY_INIT) {
          int cnt = node->init->elem_count;
          if (cnt > n) cnt = n;
          for (int i = 0; i < cnt; i++) {
            CDM_Value_t v = CDM_Deref(eval_primary(node->init->elems[i]));
            if (node->decl_type == CDM_TYPE_STRUCT && v.type == CDM_TYPE_STRUCT &&
                v.struct_id == node->struct_id && v.v.p) {
              int nf = g_struct_defs[node->struct_id].field_count;
              memcpy(block[i].v.p, v.v.p, nf * sizeof(CDM_Value_t));
              if (v.owned) value_release(v);
            } else if (node->decl_type == CDM_TYPE_STRING && v.type == CDM_TYPE_STRING) {
              block[i] = CDM_MakeString(val_to_str_raw(v));
              if (v.owned) value_release(v);
            } else {
              block[i] = v;
            }
          }
        }
        return 0;
      }
    case AST_EXPR_STMT:
      {
        CDM_Value_t v = CDM_Deref(eval_primary(node->left));
        if (v.owned) value_release(v);
        return 0;
      }
    case AST_IF:
      {
        CDM_Value_t c = CDM_Deref(eval_primary(node->left));
        int v = val_to_int_raw(c);
        if (c.owned) value_release(c);
        if (v) exec_statement(node->body);
        else if (node->right) exec_statement(node->right);
        return 0;
      }
    case AST_WHILE:
      {
        long long guard = 0;
        while (!g_returning) {
          CDM_Value_t c = CDM_Deref(eval_primary(node->left));
          int v = val_to_int_raw(c);
          if (c.owned) value_release(c);
          if (!v) break;
          exec_statement(node->body);
          if (g_break_flag) { g_break_flag = 0; break; }
          if (g_continue_flag) g_continue_flag = 0;
          if (++guard > 1000000LL) break;
        }
        g_break_flag = 0; g_continue_flag = 0;
        return 0;
      }
    case AST_DO_WHILE:
      {
        long long guard = 0;
        int cond = 1;
        while (!g_returning && cond) {
          g_continue_flag = 0;
          exec_statement(node->body);
          if (g_break_flag) { g_break_flag = 0; break; }
          CDM_Value_t c = CDM_Deref(eval_primary(node->left));
          cond = val_to_int_raw(c);
          if (c.owned) value_release(c);
          if (++guard > 1000000LL) break;
        }
        g_break_flag = 0; g_continue_flag = 0;
        return 0;
      }
    case AST_FOR:
      {
        scope_enter();
        if (node->init) exec_statement(node->init);
        long long guard = 0;
        while (!g_returning) {
          if (node->left) {
            CDM_Value_t c = CDM_Deref(eval_primary(node->left));
            int v = val_to_int_raw(c);
            if (c.owned) value_release(c);
            if (!v) break;
          }
          exec_statement(node->body);
          if (g_break_flag) { g_break_flag = 0; break; }
          if (g_continue_flag) g_continue_flag = 0;
          if (node->update) {
            CDM_Value_t u = CDM_Deref(eval_primary(node->update));
            if (u.owned) value_release(u);
          }
          if (++guard > 1000000LL) break;
        }
        g_break_flag = 0; g_continue_flag = 0;
        scope_exit();
        return 0;
      }
    case AST_RANGE_FOR:
      {
        CDM_Value_t arr = CDM_Deref(node->left ? eval_primary(node->left) : make_int(0));
        if (arr.type != CDM_TYPE_ARRAY || !arr.v.p) return 0;
        scope_enter();
        Symbol_t* s = find_symbol(node->name);
        if (!s) s = create_symbol(node->name);
        if (!s) { scope_exit(); return 0; }
        long long guard = 0;
        for (int i = 0; i < arr.array_len; i++) {
          if (g_returning || g_break_flag) break;
          g_continue_flag = 0;
          value_release(s->value);
          s->value = ((CDM_Value_t*)arr.v.p)[i];
          s->value.owned = 0;
          exec_statement(node->body);
          if (g_break_flag) break;
          if (++guard > 1000000LL) break;
        }
        g_break_flag = 0; g_continue_flag = 0;
        scope_exit();
        return 0;
      }
    case AST_SWITCH:
      {
        CDM_Value_t sw_v = CDM_Deref(eval_primary(node->left));
        long long sw = val_to_int_raw(sw_v);
        if (sw_v.owned) value_release(sw_v);

        ASTNode_t* body = node->body;
        ASTNode_t* c = body ? body->first_child : NULL;
        ASTNode_t* start = NULL;
        ASTNode_t* default_case = NULL;

        ASTNode_t* scan = c;
        while (scan) {
          if (scan->type == AST_CASE) {
            CDM_Value_t cv = CDM_Deref(eval_primary(scan->left));
            long long cval = val_to_int_raw(cv);
            if (cv.owned) value_release(cv);
            if (cval == sw) { start = scan; break; }
          } else if (scan->type == AST_DEFAULT) {
            default_case = scan;
          }
          scan = scan->next_sibling;
        }
        if (!start) start = default_case;
        if (!start) return 0;

        while (start && !g_returning && !g_break_flag) {
          ASTNode_t* s = start->first_child;
          while (s && !g_returning && !g_break_flag) {
            exec_statement(s);
            s = s->next_sibling;
          }
          start = start->next_sibling;
        }
        g_break_flag = 0;
        return 0;
      }
    case AST_CASE:
    case AST_DEFAULT:
      {
        ASTNode_t* s = node->first_child;
        while (s && !g_returning && !g_break_flag) {
          exec_statement(s);
          s = s->next_sibling;
        }
        return 0;
      }
    case AST_DELETE:
      {
        CDM_Value_t p = CDM_Deref(node->left ? eval_primary(node->left) : make_int(0));
        if (p.type == CDM_TYPE_PTR && p.v.p) free(p.v.p);
        if (p.owned) value_release(p);
        return 0;
      }
    case AST_DELETE_ARRAY:
      {
        CDM_Value_t p = CDM_Deref(node->left ? eval_primary(node->left) : make_int(0));
        if (p.type == CDM_TYPE_PTR && p.v.p) {
          if (!array_free(p.v.p)) free(p.v.p);
        }
        if (p.owned) value_release(p);
        return 0;
      }
    case AST_STRUCT_DEF: return 0;
    case AST_ENUM_DECL:  return 0;
    case AST_FUNC_DECL:  return 0;
    default: {
      CDM_Value_t v = eval_primary(node);
      if (v.owned) value_release(v);
      return 0;
    }
  }
}

int CDM_Execute(ASTNode_t* root) {
  if (!root) return -1;
  if (CDM_GetErrorCount() > 0) return -2;

  clock_t t_start = clock();

  clear_ast_cache(root);
  g_current_line = 0;

  g_symbol_count = 0;
  g_scope_depth = 0;
  g_returning = 0;
  g_return_value = make_int(0);
  g_break_flag = 0;
  g_continue_flag = 0;
  collect_user_funcs(root);

  int ret = 0;
  ASTNode_t* c = root->first_child;
  while (c) {
    if (c->type == AST_FUNC_DECL && strcmp(c->name, "main") == 0) {
      exec_statement(c->body);
      ret = (int)val_to_int_raw(g_return_value);
      break;
    }
    c = c->next_sibling;
  }

  clock_t t_end = clock();
  double elapsed = (double)(t_end - t_start) / (double)CLOCKS_PER_SEC;
  printf("exit after %.3fs returned %d\n", elapsed, ret);

  return ret;
}