#ifndef CDM_PREPROC_H
#define CDM_PREPROC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef char* (*CDM_IncludeResolver_t)(const char* filename);

void CDM_SetIncludeResolver(CDM_IncludeResolver_t fn);

void CDM_ResetIncludes(void);

char* CDM_Preprocess(const char* src);

#ifdef __cplusplus
}
#endif

#endif