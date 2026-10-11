#include "CDM_ArduinoBridge.h"
#include <Arduino.h>
#include <stdlib.h>
#include <string.h>

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

// ★ 新增
static int ar_delayMicroseconds(int argc, const char** argv) {
    if (argc < 1) return -1;
    delayMicroseconds((unsigned int)arg_int(argc, argv, 0));
    return 0;
}

// ★ 新增：Arduino map() 整数版
static int ar_map(int argc, const char** argv) {
    if (argc < 5) return 0;
    long x      = (long)arg_int(argc, argv, 0);
    long in_min = (long)arg_int(argc, argv, 1);
    long in_max = (long)arg_int(argc, argv, 2);
    long out_min= (long)arg_int(argc, argv, 3);
    long out_max= (long)arg_int(argc, argv, 4);
    return (int)map(x, in_min, in_max, out_min, out_max);
}

// ★ 新增：constrain
static int ar_constrain(int argc, const char** argv) {
    if (argc < 3) return 0;
    long x  = (long)arg_int(argc, argv, 0);
    long lo = (long)arg_int(argc, argv, 1);
    long hi = (long)arg_int(argc, argv, 2);
    if (x < lo) return (int)lo;
    if (x > hi) return (int)hi;
    return (int)x;
}

// 读取 int
static int ar_read_int(int argc, const char** argv) {
    (void)argc; (void)argv;
    while (Serial.available() <= 0) delay(10);
    return Serial.parseInt();
}

// 读取 float（把 bit pattern 装进 int 返回）
static int ar_read_float(int argc, const char** argv) {
    (void)argc; (void)argv;
    while (Serial.available() <= 0) delay(10);
    float f = Serial.parseFloat();
    int32_t bits;
    memcpy(&bits, &f, sizeof(bits));
    return (int)bits;
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
    CDM_RegisterHostFunc("__read_int",   ar_read_int);
    CDM_RegisterHostFunc("__read_float", ar_read_float);

    // ★ 新增
    CDM_RegisterHostFunc("delayMicroseconds", ar_delayMicroseconds);
    CDM_RegisterHostFunc("map",               ar_map);
    CDM_RegisterHostFunc("constrain",         ar_constrain);
}