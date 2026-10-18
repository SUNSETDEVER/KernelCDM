#include "CDM_ArduinoBridge.h"
#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int arg_int(int argc, const char** argv, int idx) {
    if (idx < 0 || idx >= argc || !argv[idx]) return 0;
    return atoi(argv[idx]);
}

static int ar_pinMode(int argc, const char** argv)      { if (argc<2) return -1; pinMode(arg_int(argc,argv,0),arg_int(argc,argv,1)); return 0; }
static int ar_digitalWrite(int argc, const char** argv) { if (argc<2) return -1; digitalWrite(arg_int(argc,argv,0),arg_int(argc,argv,1)); return 0; }
static int ar_digitalRead(int argc, const char** argv)  { if (argc<1) return 0; return digitalRead(arg_int(argc,argv,0)); }
static int ar_analogWrite(int argc, const char** argv)  { if (argc<2) return -1; analogWrite(arg_int(argc,argv,0),arg_int(argc,argv,1)); return 0; }
static int ar_analogRead(int argc, const char** argv)   { if (argc<1) return 0; return analogRead(arg_int(argc,argv,0)); }
static int ar_delay(int argc, const char** argv)        { if (argc<1) return -1; delay((unsigned long)arg_int(argc,argv,0)); return 0; }
static int ar_millis(int argc, const char** argv)       { (void)argc; (void)argv; return (int)millis(); }
static int ar_micros(int argc, const char** argv)       { (void)argc; (void)argv; return (int)micros(); }

static int ar_delayMicroseconds(int argc, const char** argv) {
    if (argc < 1) return -1;
    delayMicroseconds((unsigned int)arg_int(argc, argv, 0));
    return 0;
}

static int ar_map(int argc, const char** argv) {
    if (argc < 5) return 0;
    return (int)map((long)arg_int(argc, argv, 0),
                    (long)arg_int(argc, argv, 1),
                    (long)arg_int(argc, argv, 2),
                    (long)arg_int(argc, argv, 3),
                    (long)arg_int(argc, argv, 4));
}

static int ar_constrain(int argc, const char** argv) {
    if (argc < 3) return 0;
    long x  = (long)arg_int(argc, argv, 0);
    long lo = (long)arg_int(argc, argv, 1);
    long hi = (long)arg_int(argc, argv, 2);
    if (x < lo) return (int)lo;
    if (x > hi) return (int)hi;
    return (int)x;
}

static char g_read_str_buf[128];

static int ar_read_str(int argc, const char** argv) {
    (void)argc; (void)argv;
    int i = 0;

    while (!Serial.available()) delay(10);

    unsigned long last_char_ms = millis();
    while (i < 127) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '\n' || c == '\r') {
                if (c == '\r') {
                    delay(1);
                    if (Serial.available() && Serial.peek() == '\n') Serial.read();
                }
                break;
            }
            g_read_str_buf[i++] = c;
            last_char_ms = millis();
        } else {
            if (millis() - last_char_ms > 5000) break;
            delay(1);
        }
    }
    g_read_str_buf[i] = '\0';
    return (int)(intptr_t)g_read_str_buf;
}

static int ar_read_int(int argc, const char** argv) {
    (void)argc; (void)argv;
    while (Serial.available() <= 0) delay(10);
    return Serial.parseInt();
}

static int ar_read_float(int argc, const char** argv) {
    (void)argc; (void)argv;
    while (Serial.available() <= 0) delay(10);
    float f = Serial.parseFloat();
    int32_t bits;
    memcpy(&bits, &f, sizeof(bits));
    return (int)bits;
}

static CDM_Value_t mk_int_val(int64_t x) {
    CDM_Value_t v;
    v.type = CDM_TYPE_INT;
    v.struct_id = -1;
    v.array_len = 0; v.array_dim = 0;
    v.array_sizes[0] = v.array_sizes[1] = v.array_sizes[2] = v.array_sizes[3] = 0;
    v.array_elem_type = CDM_TYPE_INT;
    v.array_elem_struct_id = -1;
    v.owned = 0; v.from_new = 0;
    v.v.i = x;
    return v;
}

static CDM_Value_t mk_float_val(double x) {
    CDM_Value_t v;
    v.type = CDM_TYPE_FLOAT;
    v.struct_id = -1;
    v.array_len = 0; v.array_dim = 0;
    v.array_sizes[0] = v.array_sizes[1] = v.array_sizes[2] = v.array_sizes[3] = 0;
    v.array_elem_type = CDM_TYPE_INT;
    v.array_elem_struct_id = -1;
    v.owned = 0; v.from_new = 0;
    v.v.f = x;
    return v;
}

static CDM_Value_t ar_scanf(int argc, CDM_Value_t* argv) {
    if (argc < 1) return mk_int_val(0);

    while (!Serial.available()) delay(10);
    String line = Serial.readStringUntil('\n');
    line.trim();

    CDM_Value_t f = CDM_Deref(argv[0]);
    const char* fmt = "";
    if (f.type == CDM_TYPE_STRING && f.v.p) fmt = (const char*)f.v.p;

    int arg_idx = 1;
    int count = 0;
    const char* p = fmt;
    const char* s = line.c_str();

    while (*p && arg_idx < argc) {
        if (*p == '%') {
            p++;
            while (*p && isdigit((unsigned char)*p)) p++;
            while (*p && (*p == 'h' || *p == 'l' || *p == 'L')) p++;
            char spec = *p;
            if (!spec) break;

            while (*s == ' ' || *s == '\t') s++;
            if (!*s) break;

            if (spec == 'd' || spec == 'i') {
                char* end;
                long v = strtol(s, &end, 10);
                if (end == s) break;
                s = end;
                if (CDM_IsRef(argv[arg_idx])) {
                    CDM_AssignRef(argv[arg_idx], mk_int_val((int64_t)v));
                }
                count++;
            } else if (spec == 'f' || spec == 'g' || spec == 'e') {
                char* end;
                double v = strtod(s, &end);
                if (end == s) break;
                s = end;
                if (CDM_IsRef(argv[arg_idx])) {
                    CDM_AssignRef(argv[arg_idx], mk_float_val(v));
                }
                count++;
            } else if (spec == 's') {
                int n = 0;
                while (s[n] && s[n] != ' ' && s[n] != '\t') n++;
                if (n == 0) break;
                char* buf = (char*)malloc(n + 1);
                if (buf) {
                    memcpy(buf, s, n);
                    buf[n] = '\0';
                    if (CDM_IsRef(argv[arg_idx])) {
                        CDM_AssignRef(argv[arg_idx], CDM_MakeStringOwned(buf));
                    } else {
                        free(buf);
                    }
                }
                s += n;
                count++;
            } else if (spec == 'c') {
                char c = *s++;
                if (CDM_IsRef(argv[arg_idx])) {
                    char tmp[2] = { c, '\0' };
                    CDM_AssignRef(argv[arg_idx], CDM_MakeString(tmp));
                }
                count++;
            }
            p++;
            arg_idx++;
        } else {
            p++;
        }
    }

    return mk_int_val(count);
}

void CDM_ArduinoBridge_Init(void) {
    CDM_RegisterHostFunc("pinMode",      ar_pinMode);
    CDM_RegisterHostFunc("digitalWrite", ar_digitalWrite);
    CDM_RegisterHostFunc("digitalRead",  ar_digitalRead);
    CDM_RegisterHostFunc("analogWrite",  ar_analogWrite);
    CDM_RegisterHostFunc("analogRead",   ar_analogRead);
    CDM_RegisterHostFunc("delay",        ar_delay);
    CDM_RegisterHostFunc("millis",       ar_millis);
    CDM_RegisterHostFunc("micros",       ar_micros);
    CDM_RegisterHostFunc("delayMicroseconds", ar_delayMicroseconds);
    CDM_RegisterHostFunc("map",               ar_map);
    CDM_RegisterHostFunc("constrain",         ar_constrain);

    CDM_RegisterHostFunc("__read_int",   ar_read_int);
    CDM_RegisterHostFunc("__read_float", ar_read_float);
    CDM_RegisterHostFunc("__read_str",   ar_read_str);

    CDM_RegisterHostFuncEx("scanf", ar_scanf);
}