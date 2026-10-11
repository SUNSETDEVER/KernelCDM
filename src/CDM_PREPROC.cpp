#include "CDM_PREPROC.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

// ============================================================
//  配置
// ============================================================
#define PP_MAX_MACROS   64
#define PP_MAX_NAME     64
#define PP_MAX_VALUE    256
#define PP_MAX_DEPTH    8
#define PP_MAX_OUT      (64 * 1024)
#define PP_MAX_COND     16
#define PP_MAX_INCLUDED 32

// ============================================================
//  状态
// ============================================================
typedef struct {
    char name[PP_MAX_NAME];
    char value[PP_MAX_VALUE];
} PP_Macro_t;

typedef struct {
    PP_Macro_t macros[PP_MAX_MACROS];
    int        macro_count;
    char*      out;
    int        out_len;
    int        out_cap;
    int        cond_stack[PP_MAX_COND];   // 1 = 激活，0 = 跳过
    int        cond_depth;
} PP_State_t;

// ---- 全局 include 回调 ----
static CDM_IncludeResolver_t g_include_resolver = NULL;

// ---- 已 include 文件表（#pragma once 效果）----
static char g_included[PP_MAX_INCLUDED][PP_MAX_NAME];
static int  g_included_count = 0;

void CDM_SetIncludeResolver(CDM_IncludeResolver_t fn) {
    g_include_resolver = fn;
}

void CDM_ResetIncludes(void) {
    g_included_count = 0;
}

// ============================================================
//  输出缓冲
// ============================================================
static void out_init(PP_State_t* s) {
    s->out_cap = PP_MAX_OUT;
    s->out = (char*)malloc(s->out_cap);
    if (!s->out) { s->out_len = 0; s->out_cap = 0; return; }
    s->out_len = 0;
    s->out[0] = '\0';
}

static void out_append(PP_State_t* s, const char* txt, int len) {
    if (!s->out || !txt) return;
    if (len < 0) len = (int)strlen(txt);
    if (len == 0) return;
    int remain = s->out_cap - s->out_len - 1;
    if (remain <= 0) return;
    if (len > remain) len = remain;
    memcpy(s->out + s->out_len, txt, len);
    s->out_len += len;
    s->out[s->out_len] = '\0';
}

static void out_append_ch(PP_State_t* s, char c) {
    if (!s->out) return;
    if (s->out_len + 1 >= s->out_cap) return;
    s->out[s->out_len++] = c;
    s->out[s->out_len] = '\0';
}

// ============================================================
//  宏表
// ============================================================
static int pp_find(PP_State_t* s, const char* name) {
    for (int i = 0; i < s->macro_count; i++)
        if (strcmp(s->macros[i].name, name) == 0) return i;
    return -1;
}

static int pp_defined(PP_State_t* s, const char* name) {
    return pp_find(s, name) >= 0;
}

static void pp_define(PP_State_t* s, const char* name, const char* value) {
    int idx = pp_find(s, name);
    if (idx < 0) {
        if (s->macro_count >= PP_MAX_MACROS) return;
        idx = s->macro_count++;
    }
    strncpy(s->macros[idx].name, name, PP_MAX_NAME - 1);
    s->macros[idx].name[PP_MAX_NAME - 1] = '\0';
    strncpy(s->macros[idx].value, value ? value : "", PP_MAX_VALUE - 1);
    s->macros[idx].value[PP_MAX_VALUE - 1] = '\0';
}

static void pp_undef(PP_State_t* s, const char* name) {
    int idx = pp_find(s, name);
    if (idx < 0) return;
    for (int i = idx; i < s->macro_count - 1; i++)
        s->macros[i] = s->macros[i + 1];
    s->macro_count--;
}

// ============================================================
//  条件栈
// ============================================================
static int pp_active(PP_State_t* s) {
    for (int i = 0; i < s->cond_depth; i++)
        if (!s->cond_stack[i]) return 0;
    return 1;
}

// ============================================================
//  词法辅助
// ============================================================
static const char* parse_word(const char* p, char* out, int out_size) {
    int j = 0;
    while (*p && (isalnum((unsigned char)*p) || *p == '_') && j < out_size - 1)
        out[j++] = *p++;
    out[j] = '\0';
    while (*p == ' ' || *p == '\t') p++;
    return p;
}

// ============================================================
//  宏展开
// ============================================================
static void expand_line(PP_State_t* s, const char* line, int depth) {
    if (depth > PP_MAX_DEPTH) {
        out_append(s, line, -1);
        return;
    }
    const char* p = line;
    while (*p) {
        if (*p == '"') {
            out_append_ch(s, *p++);
            while (*p && *p != '"') {
                if (*p == '\\' && *(p + 1)) {
                    out_append_ch(s, *p++);
                    out_append_ch(s, *p++);
                } else {
                    out_append_ch(s, *p++);
                }
            }
            if (*p == '"') out_append_ch(s, *p++);
            continue;
        }
        if (*p == '\'') {
            out_append_ch(s, *p++);
            while (*p && *p != '\'') {
                if (*p == '\\' && *(p + 1)) {
                    out_append_ch(s, *p++);
                    out_append_ch(s, *p++);
                } else {
                    out_append_ch(s, *p++);
                }
            }
            if (*p == '\'') out_append_ch(s, *p++);
            continue;
        }
        if (*p == '/' && *(p + 1) == '/') {
            out_append(s, p, -1);
            return;
        }
        if (*p == '/' && *(p + 1) == '*') {
            out_append_ch(s, *p++);
            out_append_ch(s, *p++);
            while (*p) {
                if (*p == '*' && *(p + 1) == '/') {
                    out_append_ch(s, *p++);
                    out_append_ch(s, *p++);
                    break;
                }
                out_append_ch(s, *p++);
            }
            continue;
        }
        if (isalpha((unsigned char)*p) || *p == '_') {
            char name[PP_MAX_NAME];
            int j = 0;
            while ((isalnum((unsigned char)*p) || *p == '_') && j < PP_MAX_NAME - 1)
                name[j++] = *p++;
            name[j] = '\0';
            int idx = pp_find(s, name);
            if (idx >= 0) {
                expand_line(s, s->macros[idx].value, depth + 1);
            } else {
                out_append(s, name, -1);
            }
            continue;
        }
        out_append_ch(s, *p++);
    }
}

// ============================================================
//  前向声明
// ============================================================
static void pp_process(PP_State_t* s, const char* src, int depth);

// ============================================================
//  处理 #include
// ============================================================
static int handle_include(PP_State_t* s, const char* rest,
                          const char* line_end, int depth) {
    if (!pp_active(s)) return 0;

    const char* q = rest;
    while (q < line_end && (*q == ' ' || *q == '\t')) q++;
    if (q >= line_end) {
        printf("[PP] include: missing filename\n");
        return -1;
    }

    char filename[PP_MAX_NAME];
    int j = 0;
    char closer = 0;

    if (*q == '"')       { closer = '"'; q++; }
    else if (*q == '<')  { closer = '>'; q++; }
    else {
        printf("[PP] include: expected \"...\" or <...>\n");
        return -1;
    }

    while (q < line_end && *q != closer && j < PP_MAX_NAME - 1)
        filename[j++] = *q++;
    filename[j] = '\0';

    if (j == 0) {
        printf("[PP] include: empty filename\n");
        return -1;
    }

    for (int i = 0; i < g_included_count; i++) {
        if (strcmp(g_included[i], filename) == 0) return 0;
    }

    if (!g_include_resolver) {
        printf("[PP] include: no resolver set, skipping '%s'\n", filename);
        return -1;
    }

    if (depth >= PP_MAX_DEPTH) {
        printf("[PP] include: depth limit reached, skipping '%s'\n", filename);
        return -1;
    }

    char* content = g_include_resolver(filename);
    if (!content) {
        printf("[PP] include: failed to load '%s'\n", filename);
        return -1;
    }

    pp_process(s, content, depth + 1);
    free(content);

    if (g_included_count < PP_MAX_INCLUDED) {
        strncpy(g_included[g_included_count], filename, PP_MAX_NAME - 1);
        g_included[g_included_count][PP_MAX_NAME - 1] = '\0';
        g_included_count++;
    } else {
        printf("[PP] include: table full, no more tracking\n");
    }
    return 0;
}

// ============================================================
//  处理一段源码
// ============================================================
static void pp_process(PP_State_t* s, const char* src, int depth) {
    if (depth > PP_MAX_DEPTH) {
        out_append(s, "/* include depth exceeded */\n", -1);
        return;
    }

    const char* p = src;
    while (*p) {
        const char* line_start = p;
        while (*p && *p != '\n') p++;
        int line_len = (int)(p - line_start);
        if (*p == '\n') p++;

        const char* q = line_start;
        const char* line_end = line_start + line_len;
        while (q < line_end && (*q == ' ' || *q == '\t')) q++;

        if (q < line_end && *q == '#') {
            q++;
            while (q < line_end && (*q == ' ' || *q == '\t')) q++;

            char cmd[32];
            const char* rest = parse_word(q, cmd, sizeof(cmd));

            // ---------------- #define ----------------
            if (strcmp(cmd, "define") == 0) {
                if (pp_active(s)) {
                    char name[PP_MAX_NAME];
                    const char* v = parse_word(rest, name, sizeof(name));
                    int vlen = (int)(line_end - v);
                    while (vlen > 0 && (v[vlen-1] == ' ' || v[vlen-1] == '\t' || v[vlen-1] == '\r'))
                        vlen--;
                    char val[PP_MAX_VALUE];
                    if (vlen > PP_MAX_VALUE - 1) vlen = PP_MAX_VALUE - 1;
                    if (vlen > 0) memcpy(val, v, vlen);
                    val[vlen] = '\0';
                    pp_define(s, name, val);
                }
                out_append_ch(s, '\n');
            }
            // ---------------- #undef ----------------
            else if (strcmp(cmd, "undef") == 0) {
                if (pp_active(s)) {
                    char name[PP_MAX_NAME];
                    parse_word(rest, name, sizeof(name));
                    pp_undef(s, name);
                }
                out_append_ch(s, '\n');
            }
            // ---------------- #ifdef / #ifndef ----------------
            else if (strcmp(cmd, "ifdef") == 0 || strcmp(cmd, "ifndef") == 0) {
                char name[PP_MAX_NAME];
                parse_word(rest, name, sizeof(name));
                int def = pp_defined(s, name);
                int cond = (cmd[2] == 'n') ? !def : def;
                if (!pp_active(s)) cond = 0;
                if (s->cond_depth < PP_MAX_COND)
                    s->cond_stack[s->cond_depth++] = cond;
                out_append_ch(s, '\n');
            }
            // ---------------- #elif ----------------
            else if (strcmp(cmd, "elif") == 0) {
                if (s->cond_depth > 0) {
                    int parent_active = 1;
                    for (int i = 0; i < s->cond_depth - 1; i++)
                        if (!s->cond_stack[i]) { parent_active = 0; break; }

                    if (!parent_active) {
                        /* 父不激活，保持 0 */
                    } else if (s->cond_stack[s->cond_depth - 1]) {
                        /* 前面分支已经命中，关掉 */
                        s->cond_stack[s->cond_depth - 1] = 0;
                    } else {
                        /* 评估新条件：#elif defined(NAME) 或 #elif NAME */
                        const char* q2 = rest;
                        while (q2 < line_end && (*q2 == ' ' || *q2 == '\t')) q2++;
                        if (strncmp(q2, "defined", 7) == 0) {
                            q2 += 7;
                            while (q2 < line_end && (*q2 == ' ' || *q2 == '\t')) q2++;
                            if (q2 < line_end && *q2 == '(') q2++;
                            while (q2 < line_end && (*q2 == ' ' || *q2 == '\t')) q2++;
                        }
                        char name[PP_MAX_NAME];
                        int j = 0;
                        while (q2 < line_end && (isalnum((unsigned char)*q2) || *q2 == '_') && j < PP_MAX_NAME - 1)
                            name[j++] = *q2++;
                        name[j] = '\0';
                        int def = pp_defined(s, name);
                        s->cond_stack[s->cond_depth - 1] = def;
                    }
                }
                out_append_ch(s, '\n');
            }
            // ---------------- #else ----------------
            else if (strcmp(cmd, "else") == 0) {
                if (s->cond_depth > 0) {
                    int parent_active = 1;
                    for (int i = 0; i < s->cond_depth - 1; i++)
                        if (!s->cond_stack[i]) { parent_active = 0; break; }
                    if (parent_active)
                        s->cond_stack[s->cond_depth - 1] = !s->cond_stack[s->cond_depth - 1];
                    else
                        s->cond_stack[s->cond_depth - 1] = 0;
                }
                out_append_ch(s, '\n');
            }
            // ---------------- #endif ----------------
            else if (strcmp(cmd, "endif") == 0) {
                if (s->cond_depth > 0) s->cond_depth--;
                out_append_ch(s, '\n');
            }
            // ---------------- #include ----------------
            else if (strcmp(cmd, "include") == 0) {
                handle_include(s, rest, line_end, depth);
                out_append_ch(s, '\n');
            }
            // ---------------- #pragma ----------------
            else if (strcmp(cmd, "pragma") == 0) {
                out_append_ch(s, '\n');
            }
            // ---------------- 其它 ----------------
            else {
                out_append_ch(s, '\n');
            }
        }
        else {
            if (pp_active(s) && line_len > 0) {
                char* tmp = (char*)malloc(line_len + 1);
                if (tmp) {
                    memcpy(tmp, line_start, line_len);
                    tmp[line_len] = '\0';
                    expand_line(s, tmp, 0);
                    free(tmp);
                }
            }
            out_append_ch(s, '\n');
        }
    }
}

// ============================================================
//  主入口
// ============================================================
char* CDM_Preprocess(const char* src) {
    CDM_ResetIncludes();

    PP_State_t* s = (PP_State_t*)calloc(1, sizeof(PP_State_t));
    if (!s) return NULL;

    out_init(s);
    if (!s->out) { free(s); return NULL; }

    pp_process(s, src, 0);

    char* result = s->out;
    free(s);
    return result;
}