#include "CDM_STD.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

// ============================================================
//  工具
// ============================================================
static double as_double(CDM_Value_t v) {
    v = CDM_Deref(v);
    if (v.type == CDM_TYPE_FLOAT) return v.v.f;
    if (v.type == CDM_TYPE_INT)   return (double)v.v.i;
    if (v.type == CDM_TYPE_STRING) return v.v.p ? atof((const char*)v.v.p) : 0.0;
    return 0.0;
}
static int64_t as_int(CDM_Value_t v) {
    v = CDM_Deref(v);
    if (v.type == CDM_TYPE_FLOAT) return (int64_t)v.v.f;
    if (v.type == CDM_TYPE_INT)   return v.v.i;
    if (v.type == CDM_TYPE_STRING) return v.v.p ? atoll((const char*)v.v.p) : 0;
    return 0;
}
static const char* as_str(CDM_Value_t v) {
    v = CDM_Deref(v);
    if (v.type == CDM_TYPE_STRING && v.v.p) return (const char*)v.v.p;
    return "";
}

static CDM_Value_t mk_f(double x) {
    CDM_Value_t v;
    v.type = CDM_TYPE_FLOAT;
    v.struct_id = -1;
    v.array_len = 0; v.array_dim = 0;
    v.array_sizes[0]=v.array_sizes[1]=v.array_sizes[2]=v.array_sizes[3]=0;
    v.array_elem_type = CDM_TYPE_INT; v.array_elem_struct_id = -1;
    v.owned = 0; v.from_new = 0; v.v.f = x;
    return v;
}
static CDM_Value_t mk_i(int64_t x) {
    CDM_Value_t v;
    v.type = CDM_TYPE_INT;
    v.struct_id = -1;
    v.array_len = 0; v.array_dim = 0;
    v.array_sizes[0]=v.array_sizes[1]=v.array_sizes[2]=v.array_sizes[3]=0;
    v.array_elem_type = CDM_TYPE_INT; v.array_elem_struct_id = -1;
    v.owned = 0; v.from_new = 0; v.v.i = x;
    return v;
}

// ============================================================
//  数学
// ============================================================
#define STD1(name, expr) \
    static CDM_Value_t std_##name(int argc, CDM_Value_t* argv) { \
        double x = argc >= 1 ? as_double(argv[0]) : 0.0; \
        (void)x; return mk_f(expr); \
    }
#define STD2(name, expr) \
    static CDM_Value_t std_##name(int argc, CDM_Value_t* argv) { \
        double a = argc >= 1 ? as_double(argv[0]) : 0.0; \
        double b = argc >= 2 ? as_double(argv[1]) : 0.0; \
        (void)a; (void)b; return mk_f(expr); \
    }

STD1(sqrt,  sqrt(x))
STD1(sin,   sin(x))
STD1(cos,   cos(x))
STD1(tan,   tan(x))
STD1(asin,  asin(x))
STD1(acos,  acos(x))
STD1(atan,  atan(x))
STD1(floor, floor(x))
STD1(ceil,  ceil(x))
STD1(round, round(x))
STD1(fabs,  fabs(x))
STD1(exp,   exp(x))
STD1(log,   log(x))
STD1(log10, log10(x))
STD1(log2,  log2(x))
STD2(atan2, atan2(a, b))
STD2(fmod,  fmod(a, b))
STD2(pow,   pow(a, b))
STD2(min,   a < b ? a : b)
STD2(max,   a > b ? a : b)

static CDM_Value_t std_abs(int argc, CDM_Value_t* argv) {
    if (argc < 1) return mk_i(0);
    CDM_Value_t v = CDM_Deref(argv[0]);
    if (v.type == CDM_TYPE_INT) {
        int64_t x = v.v.i;
        return mk_i(x < 0 ? -x : x);
    }
    return mk_f(fabs(as_double(v)));
}
static CDM_Value_t std_rand(int argc, CDM_Value_t* argv) { (void)argc; (void)argv; return mk_i(rand()); }
static CDM_Value_t std_srand(int argc, CDM_Value_t* argv) {
    if (argc >= 1) srand((unsigned)as_int(argv[0]));
    return mk_i(0);
}

// ============================================================
//  std::string 构造
// ============================================================
static CDM_Value_t std_string_ctor(int argc, CDM_Value_t* argv) {
    if (argc < 1) return CDM_MakeString("");
    CDM_Value_t v = CDM_Deref(argv[0]);
    if (v.type == CDM_TYPE_STRING) return CDM_MakeString(as_str(v));
    if (v.type == CDM_TYPE_FLOAT) {
        char buf[64]; snprintf(buf, sizeof(buf), "%g", v.v.f);
        return CDM_MakeString(buf);
    }
    if (v.type == CDM_TYPE_INT) {
        char buf[64]; snprintf(buf, sizeof(buf), "%lld", (long long)v.v.i);
        return CDM_MakeString(buf);
    }
    return CDM_MakeString("");
}

// ============================================================
//  std::string 方法
// ============================================================
static CDM_Value_t str_length(int argc, CDM_Value_t* argv) {
    if (argc < 1) return mk_i(0);
    return mk_i((int64_t)strlen(as_str(argv[0])));
}
static CDM_Value_t str_empty(int argc, CDM_Value_t* argv) {
    if (argc < 1) return mk_i(1);
    return mk_i(strlen(as_str(argv[0])) == 0);
}
static CDM_Value_t str_substr(int argc, CDM_Value_t* argv) {
    const char* s = argc >= 1 ? as_str(argv[0]) : "";
    int slen = (int)strlen(s);
    int pos = argc >= 2 ? (int)as_int(argv[1]) : 0;
    int len = argc >= 3 ? (int)as_int(argv[2]) : slen - pos;
    if (pos < 0) pos = 0;
    if (pos > slen) pos = slen;
    if (len < 0) len = 0;
    if (pos + len > slen) len = slen - pos;
    char* buf = (char*)malloc(len + 1);
    if (!buf) return CDM_MakeString("");
    memcpy(buf, s + pos, len);
    buf[len] = '\0';
    return CDM_MakeStringOwned(buf);
}
static CDM_Value_t str_find(int argc, CDM_Value_t* argv) {
    const char* s = argc >= 1 ? as_str(argv[0]) : "";
    const char* needle = argc >= 2 ? as_str(argv[1]) : "";
    const char* p = strstr(s, needle);
    return mk_i(p ? (int64_t)(p - s) : -1);
}
static CDM_Value_t str_append(int argc, CDM_Value_t* argv) {
    const char* a = argc >= 1 ? as_str(argv[0]) : "";
    const char* b = argc >= 2 ? as_str(argv[1]) : "";
    size_t la = strlen(a), lb = strlen(b);
    char* buf = (char*)malloc(la + lb + 1);
    if (!buf) return CDM_MakeString("");
    memcpy(buf, a, la);
    memcpy(buf + la, b, lb);
    buf[la + lb] = '\0';
    return CDM_MakeStringOwned(buf);
}
static CDM_Value_t str_replace(int argc, CDM_Value_t* argv) {
    const char* s = argc >= 1 ? as_str(argv[0]) : "";
    int pos = argc >= 2 ? (int)as_int(argv[1]) : 0;
    int len = argc >= 3 ? (int)as_int(argv[2]) : 0;
    const char* repl = argc >= 4 ? as_str(argv[3]) : "";
    int slen = (int)strlen(s);
    int rlen = (int)strlen(repl);
    if (pos < 0) pos = 0;
    if (pos > slen) pos = slen;
    if (len < 0) len = 0;
    if (pos + len > slen) len = slen - pos;
    int new_len = slen - len + rlen;
    char* buf = (char*)malloc(new_len + 1);
    if (!buf) return CDM_MakeString("");
    memcpy(buf, s, pos);
    memcpy(buf + pos, repl, rlen);
    memcpy(buf + pos + rlen, s + pos + len, slen - pos - len);
    buf[new_len] = '\0';
    return CDM_MakeStringOwned(buf);
}
static CDM_Value_t str_trim(int argc, CDM_Value_t* argv) {
    const char* s = argc >= 1 ? as_str(argv[0]) : "";
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
    int len = (int)strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t' || s[len-1] == '\n' || s[len-1] == '\r')) len--;
    char* buf = (char*)malloc(len + 1);
    if (!buf) return CDM_MakeString("");
    memcpy(buf, s, len);
    buf[len] = '\0';
    return CDM_MakeStringOwned(buf);
}
static CDM_Value_t str_c_str(int argc, CDM_Value_t* argv) {
    if (argc < 1) return CDM_MakeString("");
    return CDM_MakeString(as_str(argv[0]));
}
static CDM_Value_t str_at(int argc, CDM_Value_t* argv) {
    const char* s = argc >= 1 ? as_str(argv[0]) : "";
    int i = argc >= 2 ? (int)as_int(argv[1]) : 0;
    int slen = (int)strlen(s);
    if (i < 0 || i >= slen) return CDM_MakeString("");
    char buf[2] = { s[i], '\0' };
    return CDM_MakeString(buf);
}

// ============================================================
//  全局字符串函数
// ============================================================
static CDM_Value_t std_strlen(int argc, CDM_Value_t* argv) {
    if (argc < 1) return mk_i(0);
    return mk_i((int64_t)strlen(as_str(argv[0])));
}
static CDM_Value_t std_strcmp(int argc, CDM_Value_t* argv) {
    if (argc < 2) return mk_i(0);
    return mk_i(strcmp(as_str(argv[0]), as_str(argv[1])));
}
static CDM_Value_t std_strcpy(int argc, CDM_Value_t* argv) {
    if (argc < 2) return CDM_MakeString("");
    if (CDM_IsRef(argv[0])) {
        CDM_Value_t v = CDM_MakeString(as_str(argv[1]));
        CDM_AssignRef(argv[0], v);
        return v;
    }
    return CDM_MakeString(as_str(argv[1]));
}
static CDM_Value_t std_strcat(int argc, CDM_Value_t* argv) {
    if (argc < 2) return CDM_MakeString("");
    const char* a = as_str(argv[0]);
    const char* b = as_str(argv[1]);
    size_t la = strlen(a), lb = strlen(b);
    char* buf = (char*)malloc(la + lb + 1);
    if (!buf) return CDM_MakeString("");
    memcpy(buf, a, la);
    memcpy(buf + la, b, lb);
    buf[la + lb] = '\0';
    CDM_Value_t res = CDM_MakeStringOwned(buf);
    if (CDM_IsRef(argv[0])) CDM_AssignRef(argv[0], res);
    return res;
}

/* ★ strncpy(dst, src, n) —— dst 必须是 REF */
static CDM_Value_t std_strncpy(int argc, CDM_Value_t* argv) {
    if (argc < 3) return CDM_MakeString("");
    const char* src = as_str(argv[1]);
    int n = (int)as_int(argv[2]);
    if (n <= 0) return CDM_MakeString("");

    size_t sl = strlen(src);
    size_t copy_len = (sl < (size_t)n) ? sl : (size_t)n;
    char* buf = (char*)malloc(copy_len + 1);
    if (!buf) return CDM_MakeString("");
    memcpy(buf, src, copy_len);
    buf[copy_len] = '\0';

    if (CDM_IsRef(argv[0])) {
        CDM_Value_t* slot = CDM_RefPtr(argv[0]);
        if (slot && slot->type == CDM_TYPE_STRING) {
            if (slot->owned && slot->v.p) free(slot->v.p);
            *slot = CDM_MakeStringOwned(buf);
            CDM_Value_t ret = *slot; ret.owned = 0;
            return ret;
        }
    }
    return CDM_MakeStringOwned(buf);
}

/* ★ strncmp(a, b, n) */
static CDM_Value_t std_strncmp(int argc, CDM_Value_t* argv) {
    if (argc < 3) return mk_i(0);
    int n = (int)as_int(argv[2]);
    if (n < 0) n = 0;
    return mk_i(strncmp(as_str(argv[0]), as_str(argv[1]), n));
}

/* ★ strchr(s, c) —— 返回位置，找不到返回 -1 */
static CDM_Value_t std_strchr(int argc, CDM_Value_t* argv) {
    if (argc < 2) return mk_i(-1);
    const char* s = as_str(argv[0]);
    char c = (char)as_int(argv[1]);
    const char* p = strchr(s, c);
    return mk_i(p ? (int64_t)(p - s) : -1);
}

// ============================================================
//  数值转换
// ============================================================
static CDM_Value_t std_atoi(int argc, CDM_Value_t* argv) {
    return mk_i(argc >= 1 ? (int64_t)atoll(as_str(argv[0])) : 0);
}
static CDM_Value_t std_atof(int argc, CDM_Value_t* argv) {
    return mk_f(argc >= 1 ? atof(as_str(argv[0])) : 0.0);
}
static CDM_Value_t std_to_string(int argc, CDM_Value_t* argv) {
    if (argc < 1) return CDM_MakeString("");
    CDM_Value_t v = CDM_Deref(argv[0]);
    if (v.type == CDM_TYPE_STRING) return CDM_MakeString(as_str(v));
    if (v.type == CDM_TYPE_FLOAT) {
        char buf[64]; snprintf(buf, sizeof(buf), "%g", v.v.f);
        return CDM_MakeString(buf);
    }
    char buf[64]; snprintf(buf, sizeof(buf), "%lld", (long long)v.v.i);
    return CDM_MakeString(buf);
}

// ============================================================
//  格式化器
// ============================================================
static char* fmt_impl(const char* fmt, CDM_Value_t* argv, int argc, int first_arg_idx) {
    int cap = 128;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    int len = 0;
    int argi = first_arg_idx;

    for (const char* p = fmt; *p; p++) {
        if (len + 64 > cap) {
            int new_cap = cap * 2;
            char* n = (char*)realloc(out, new_cap);
            if (!n) { free(out); return NULL; }
            out = n;
            cap = new_cap;
        }
        if (*p != '%') { out[len++] = *p; continue; }
        p++;
        if (*p == '\0') { out[len++] = '%'; break; }
        if (*p == '%') { out[len++] = '%'; continue; }
        while (*p && (*p == '-' || *p == '+' || *p == ' ' || *p == '#' || *p == '0')) p++;
        if (*p == '*') p++;
        else while (*p && isdigit((unsigned char)*p)) p++;
        if (*p == '.') {
            p++;
            if (*p == '*') p++;
            else while (*p && isdigit((unsigned char)*p)) p++;
        }
        while (*p && (*p == 'h' || *p == 'l' || *p == 'L' ||
                      *p == 'q' || *p == 'j' || *p == 'z' || *p == 't')) p++;
        char spec = *p;
        if (!spec) { out[len++] = '%'; break; }

        CDM_Value_t v = (argi < argc) ? CDM_Deref(argv[argi]) : mk_i(0);
        argi++;

        char buf[64];
        int n = 0;
        switch (spec) {
            case 'd': case 'i':
                n = snprintf(buf, sizeof(buf), "%lld", (long long)as_int(v));
                break;
            case 'u':
                n = snprintf(buf, sizeof(buf), "%llu", (unsigned long long)as_int(v));
                break;
            case 'x':
                n = snprintf(buf, sizeof(buf), "%llx", (unsigned long long)as_int(v));
                break;
            case 'X':
                n = snprintf(buf, sizeof(buf), "%llX", (unsigned long long)as_int(v));
                break;
            case 'o':
                n = snprintf(buf, sizeof(buf), "%llo", (unsigned long long)as_int(v));
                break;
            case 'f': case 'e': case 'E': case 'g': case 'G':
                n = snprintf(buf, sizeof(buf), "%g", as_double(v));
                break;
            case 'c':
                n = snprintf(buf, sizeof(buf), "%c", (char)as_int(v));
                break;
            case 's': {
                const char* s = as_str(v);
                while (*s) {
                    if (len + 1 >= cap) {
                        int new_cap = cap * 2;
                        char* nn = (char*)realloc(out, new_cap);
                        if (!nn) { free(out); return NULL; }
                        out = nn;
                        cap = new_cap;
                    }
                    out[len++] = *s++;
                }
                continue;
            }
            case 'p':
                n = snprintf(buf, sizeof(buf), "%p", v.v.p);
                break;
            default:
                if (len + 2 >= cap) {
                    int new_cap = cap * 2;
                    char* nn = (char*)realloc(out, new_cap);
                    if (!nn) { free(out); return NULL; }
                    out = nn;
                    cap = new_cap;
                }
                out[len++] = '%';
                out[len++] = spec;
                continue;
        }
        if (n > 0) {
            if (len + n + 1 > cap) {
                int new_cap = len + n + 32;
                char* nn = (char*)realloc(out, new_cap);
                if (!nn) { free(out); return NULL; }
                out = nn;
                cap = new_cap;
            }
            memcpy(out + len, buf, n);
            len += n;
        }
    }
    out[len] = '\0';
    return out;
}

// ---- sprintf / snprintf / printf ----
static CDM_Value_t std_sprintf(int argc, CDM_Value_t* argv) {
    if (argc < 2) return CDM_MakeString("");
    const char* fmt = as_str(argv[1]);
    char* result = fmt_impl(fmt, argv, argc, 2);
    if (!result) return CDM_MakeString("");

    if (CDM_IsRef(argv[0])) {
        CDM_Value_t* slot = CDM_RefPtr(argv[0]);
        if (slot && slot->type == CDM_TYPE_STRING) {
            if (slot->owned && slot->v.p) free(slot->v.p);
            *slot = CDM_MakeStringOwned(result);
            CDM_Value_t ret = *slot;
            ret.owned = 0;
            return ret;
        }
    }
    return CDM_MakeStringOwned(result);
}

static CDM_Value_t std_snprintf(int argc, CDM_Value_t* argv) {
    if (argc < 3) return CDM_MakeString("");
    int maxlen = (int)as_int(argv[1]);
    if (maxlen <= 0) return CDM_MakeString("");
    const char* fmt = as_str(argv[2]);
    char* result = fmt_impl(fmt, argv, argc, 3);
    if (!result) return CDM_MakeString("");

    int rlen = (int)strlen(result);
    if (rlen >= maxlen) result[maxlen - 1] = '\0';

    if (CDM_IsRef(argv[0])) {
        CDM_Value_t* slot = CDM_RefPtr(argv[0]);
        if (slot && slot->type == CDM_TYPE_STRING) {
            if (slot->owned && slot->v.p) free(slot->v.p);
            *slot = CDM_MakeStringOwned(result);
            CDM_Value_t ret = *slot;
            ret.owned = 0;
            return ret;
        }
    }
    return CDM_MakeStringOwned(result);
}

static CDM_Value_t std_printf(int argc, CDM_Value_t* argv) {
    if (argc < 1) return mk_i(0);
    const char* fmt = as_str(argv[0]);
    char* result = fmt_impl(fmt, argv, argc, 1);
    if (!result) return mk_i(-1);
    int n = (int)strlen(result);
    fputs(result, stdout);
    free(result);
    return mk_i(n);
}

// ============================================================
//  文件 IO
//  用法：
//    auto fp = fopen("/littlefs/foo.txt", "w");
//    if (fp) { fprintf(fp, "hello %d\n", 42); fclose(fp); }
// ============================================================
static FILE* as_filep(CDM_Value_t v) {
    v = CDM_Deref(v);
    if (v.type != CDM_TYPE_PTR || !v.v.p) return NULL;
    return (FILE*)v.v.p;
}

static CDM_Value_t std_fopen(int argc, CDM_Value_t* argv) {
    if (argc < 2) return mk_i(0);
    const char* path = as_str(argv[0]);
    const char* mode = as_str(argv[1]);
    FILE* fp = fopen(path, mode);
    if (!fp) {
        printf("[fopen] failed: %s (mode=%s)\n", path, mode);
        return mk_i(0);
    }
    return CDM_MakePtr(fp, 0);   /* FILE* 由 fclose 管理，不归 CDM 释放 */
}

static CDM_Value_t std_fclose(int argc, CDM_Value_t* argv) {
    if (argc < 1) return mk_i(-1);
    FILE* fp = as_filep(argv[0]);
    if (!fp) return mk_i(-1);
    return mk_i(fclose(fp));
}

static CDM_Value_t std_fprintf(int argc, CDM_Value_t* argv) {
    if (argc < 2) return mk_i(-1);
    FILE* fp = as_filep(argv[0]);
    if (!fp) return mk_i(-1);
    const char* fmt = as_str(argv[1]);
    char* text = fmt_impl(fmt, argv, argc, 2);
    if (!text) return mk_i(-1);
    int r = fputs(text, fp);
    free(text);
    return mk_i(r);
}

static CDM_Value_t std_fputs(int argc, CDM_Value_t* argv) {
    if (argc < 2) return mk_i(-1);
    const char* s = as_str(argv[0]);
    FILE* fp = as_filep(argv[1]);
    if (!fp) return mk_i(-1);
    return mk_i(fputs(s, fp));
}

/* fgets(buf, n, fp) —— buf 是 REF（std::string） */
static CDM_Value_t std_fgets(int argc, CDM_Value_t* argv) {
    if (argc < 3) return CDM_MakeString("");
    int n = (int)as_int(argv[1]);
    if (n <= 0 || n > 4096) n = 256;
    FILE* fp = as_filep(argv[2]);
    if (!fp) return CDM_MakeString("");

    char* tmp = (char*)malloc(n);
    if (!tmp) return CDM_MakeString("");
    char* r = fgets(tmp, n, fp);
    if (!r) { free(tmp); return CDM_MakeString(""); }

    /* 去掉尾部换行 */
    size_t L = strlen(tmp);
    if (L > 0 && tmp[L-1] == '\n') tmp[L-1] = '\0';

    CDM_Value_t v = CDM_MakeStringOwned(tmp);
    if (CDM_IsRef(argv[0])) {
        CDM_Value_t* slot = CDM_RefPtr(argv[0]);
        if (slot && slot->type == CDM_TYPE_STRING) {
            if (slot->owned && slot->v.p) free(slot->v.p);
            *slot = v;
            CDM_Value_t ret = *slot;
            ret.owned = 0;
            return ret;
        }
    }
    return v;
}

static CDM_Value_t std_feof(int argc, CDM_Value_t* argv) {
    if (argc < 1) return mk_i(1);
    FILE* fp = as_filep(argv[0]);
    if (!fp) return mk_i(1);
    return mk_i(feof(fp));
}

static CDM_Value_t std_fread(int argc, CDM_Value_t* argv) {
    if (argc < 3) return mk_i(0);
    FILE* fp = as_filep(argv[2]);
    if (!fp) return mk_i(0);
    int n = (int)as_int(argv[1]);
    if (n <= 0 || n > 4096) n = 256;
    char* tmp = (char*)malloc(n + 1);
    if (!tmp) return mk_i(0);
    size_t got = fread(tmp, 1, n, fp);
    tmp[got] = '\0';
    CDM_Value_t v = CDM_MakeStringOwned(tmp);
    if (CDM_IsRef(argv[0])) {
        CDM_Value_t* slot = CDM_RefPtr(argv[0]);
        if (slot && slot->type == CDM_TYPE_STRING) {
            if (slot->owned && slot->v.p) free(slot->v.p);
            *slot = v;
            CDM_Value_t ret = *slot;
            ret.owned = 0;
            return ret;
        }
    }
    return v;
}

static CDM_Value_t std_fwrite(int argc, CDM_Value_t* argv) {
    if (argc < 2) return mk_i(0);
    const char* s = as_str(argv[0]);
    FILE* fp = as_filep(argv[1]);
    if (!fp) return mk_i(0);
    size_t n = strlen(s);
    return mk_i((int64_t)fwrite(s, 1, n, fp));
}

// ============================================================
//  注册
// ============================================================
void CDM_Std_Init(void) {
    CDM_RegisterHostFuncEx("std::sqrt",  std_sqrt);
    CDM_RegisterHostFuncEx("std::sin",   std_sin);
    CDM_RegisterHostFuncEx("std::cos",   std_cos);
    CDM_RegisterHostFuncEx("std::tan",   std_tan);
    CDM_RegisterHostFuncEx("std::asin",  std_asin);
    CDM_RegisterHostFuncEx("std::acos",  std_acos);
    CDM_RegisterHostFuncEx("std::atan",  std_atan);
    CDM_RegisterHostFuncEx("std::atan2", std_atan2);
    CDM_RegisterHostFuncEx("std::floor", std_floor);
    CDM_RegisterHostFuncEx("std::ceil",  std_ceil);
    CDM_RegisterHostFuncEx("std::round", std_round);
    CDM_RegisterHostFuncEx("std::fabs",  std_fabs);
    CDM_RegisterHostFuncEx("std::fmod",  std_fmod);
    CDM_RegisterHostFuncEx("std::pow",   std_pow);
    CDM_RegisterHostFuncEx("std::exp",   std_exp);
    CDM_RegisterHostFuncEx("std::log",   std_log);
    CDM_RegisterHostFuncEx("std::log10", std_log10);
    CDM_RegisterHostFuncEx("std::log2",  std_log2);
    CDM_RegisterHostFuncEx("std::abs",   std_abs);
    CDM_RegisterHostFuncEx("std::min",   std_min);
    CDM_RegisterHostFuncEx("std::max",   std_max);

    CDM_RegisterHostFuncEx("std::rand",       std_rand);
    CDM_RegisterHostFuncEx("std::srand",      std_srand);
    CDM_RegisterHostFuncEx("std::string",     std_string_ctor);

    CDM_RegisterHostFuncEx("std::string.length",  str_length);
    CDM_RegisterHostFuncEx("std::string.size",    str_length);
    CDM_RegisterHostFuncEx("std::string.empty",   str_empty);
    CDM_RegisterHostFuncEx("std::string.substr",  str_substr);
    CDM_RegisterHostFuncEx("std::string.find",    str_find);
    CDM_RegisterHostFuncEx("std::string.append",  str_append);
    CDM_RegisterHostFuncEx("std::string.replace", str_replace);
    CDM_RegisterHostFuncEx("std::string.trim",    str_trim);
    CDM_RegisterHostFuncEx("std::string.c_str",   str_c_str);
    CDM_RegisterHostFuncEx("std::string.at",      str_at);

    CDM_RegisterHostFuncEx("std::strlen",     std_strlen);
    CDM_RegisterHostFuncEx("std::strcmp",     std_strcmp);
    CDM_RegisterHostFuncEx("std::strcpy",     std_strcpy);
    CDM_RegisterHostFuncEx("std::strcat",     std_strcat);
    CDM_RegisterHostFuncEx("std::atoi",       std_atoi);
    CDM_RegisterHostFuncEx("std::atof",       std_atof);
    CDM_RegisterHostFuncEx("std::to_string",  std_to_string);

    CDM_RegisterHostFuncEx("sprintf",   std_sprintf);
    CDM_RegisterHostFuncEx("snprintf",  std_snprintf);
    CDM_RegisterHostFuncEx("printf",    std_printf);
    CDM_RegisterHostFuncEx("std::sprintf",  std_sprintf);
    CDM_RegisterHostFuncEx("std::snprintf", std_snprintf);
    CDM_RegisterHostFuncEx("std::printf",   std_printf);
    CDM_RegisterHostFuncEx("std::strncpy",    std_strncpy);
    CDM_RegisterHostFuncEx("std::strncmp",    std_strncmp);
    CDM_RegisterHostFuncEx("std::strchr",     std_strchr);
    CDM_RegisterHostFuncEx("strncpy",         std_strncpy);
    CDM_RegisterHostFuncEx("strncmp",         std_strncmp);
    CDM_RegisterHostFuncEx("strchr",          std_strchr);

    CDM_RegisterHostFuncEx("fopen",    std_fopen);
    CDM_RegisterHostFuncEx("fclose",   std_fclose);
    CDM_RegisterHostFuncEx("fprintf",  std_fprintf);
    CDM_RegisterHostFuncEx("fputs",    std_fputs);
    CDM_RegisterHostFuncEx("fgets",    std_fgets);
    CDM_RegisterHostFuncEx("feof",     std_feof);
    CDM_RegisterHostFuncEx("fread",    std_fread);
    CDM_RegisterHostFuncEx("fwrite",   std_fwrite);
    CDM_RegisterHostFuncEx("std::fopen",   std_fopen);
    CDM_RegisterHostFuncEx("std::fclose",  std_fclose);
    CDM_RegisterHostFuncEx("std::fprintf", std_fprintf);
    CDM_RegisterHostFuncEx("std::fputs",   std_fputs);
    CDM_RegisterHostFuncEx("std::fgets",   std_fgets);
    CDM_RegisterHostFuncEx("std::feof",    std_feof);
    CDM_RegisterHostFuncEx("std::fread",   std_fread);
    CDM_RegisterHostFuncEx("std::fwrite",  std_fwrite);
}