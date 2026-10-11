// ============================================================
//  CMScript - File I/O Example
// ------------------------------------------------------------
//  Shows how a script can read and write files through the
//  std library functions:
//
//      fopen(path, mode)         -> FILE*  (0 on failure)
//      fclose(fp)
//      fprintf(fp, fmt, ...)     -> int
//      fputs(str, fp)            -> int
//      fgets(buf, n, fp)         -> string (writes into buf too)
//      fread(buf, n, fp)         -> string
//      fwrite(str, fp)           -> int
//      feof(fp)                  -> 0 or 1
//
//  IMPORTANT: fopen() uses the C library's VFS path. On ESP32(For Example)
//  this means you MUST mount a filesystem first and use the
//  mount-point prefix in the path.
//
//      LittleFS  ->  /littlefs/foo.txt
//      SPIFFS    ->  /spiffs/foo.txt
//      SD        ->  /sd/foo.txt
//
//  If you want to run this example on a PC (Arduino IDE's
//  desktop build), use a path like "./foo.txt" instead.
// ============================================================

#include <Arduino.h>
#include <CMScript.h>

// ------------------------------------------------------------
//  Uncomment ONE of the following to enable a filesystem.
//  If none is enabled, we fall back to "./" (works on PC).
// ------------------------------------------------------------

// #define USE_LITTLEFS
// #define USE_SPIFFS
// #define USE_SD

#if defined(USE_LITTLEFS)
    #include <LittleFS.h>
    #define FS_PREFIX  "/littlefs/"
    #define MOUNT()    LittleFS.begin(true)
    #define UMOUNT()   LittleFS.end()
#elif defined(USE_SPIFFS)
    #include <SPIFFS.h>
    #define FS_PREFIX  "/spiffs/"
    #define MOUNT()    SPIFFS.begin(true)
    #define UMOUNT()   SPIFFS.end()
#elif defined(USE_SD)
    #include <SD.h>
    #include <SPI.h>
    #define FS_PREFIX  "/sd/"
    #define SD_CS_PIN  5
    #define MOUNT()    SD.begin(SD_CS_PIN)
    #define UMOUNT()   SD.end()
#else
    #define FS_PREFIX  "./"
    #define MOUNT()    true
    #define UMOUNT()   ((void)0)
#endif

// ------------------------------------------------------------
//  Print callback
// ------------------------------------------------------------
static int serial_print(int argc, const char** argv) {
    if (argc >= 1) printf("%s", argv[0]);
    return 0;
}

// ------------------------------------------------------------
//  Script source
// ------------------------------------------------------------
//  The script uses a FILE_PATH macro that we inject from the
//  host. This way we can change the path from C++ without
//  editing the script.
//
//  To see exactly what the preprocessor produces, flip
//  DUMP_PREPROCESSED to 1 below.
// ------------------------------------------------------------
static const char* src =
    "#define FILE_PATH \"" FS_PREFIX "test.txt\"\n"
    "\n"
    "int main() {\n"
    "    // --------------------------------------------------\n"
    "    // Part 1: write to a file\n"
    "    // --------------------------------------------------\n"
    "    std::cout << \"-- writing --\\n\";\n"
    "\n"
    "    auto fp = fopen(FILE_PATH, \"w\");\n"
    "    if (fp == 0) {\n"
    "        std::cout << \"open for write failed\\n\";\n"
    "        return 1;\n"
    "    }\n"
    "\n"
    "    fprintf(fp, \"line 1: %d\\n\", 42);\n"
    "    fprintf(fp, \"line 2: %s\\n\", \"hello\");\n"
    "    fprintf(fp, \"line 3: %.2f\\n\", 3.14);\n"
    "    fputs(\"line 4: plain text\\n\", fp);\n"
    "\n"
    "    fclose(fp);\n"
    "    std::cout << \"closed.\\n\";\n"
    "\n"
    "    // --------------------------------------------------\n"
    "    // Part 2: read it back line by line\n"
    "    // --------------------------------------------------\n"
    "    std::cout << \"-- reading --\\n\";\n"
    "\n"
    "    auto fp2 = fopen(FILE_PATH, \"r\");\n"
    "    if (fp2 == 0) {\n"
    "        std::cout << \"open for read failed\\n\";\n"
    "        return 2;\n"
    "    }\n"
    "\n"
    "    std::string line = \"\";\n"
    "    int count = 0;\n"
    "    while (!feof(fp2)) {\n"
    "        line = fgets(line, 128, fp2);\n"
    "        if (line.length() > 0) {\n"
    "            count++;\n"
    "            std::cout << \"[\" << count << \"] \" << line << \"\\n\";\n"
    "        }\n"
    "    }\n"
    "    fclose(fp2);\n"
    "\n"
    "    std::cout << \"read \" << count << \" lines.\\n\";\n"
    "\n"
    "    // --------------------------------------------------\n"
    "    // Part 3: append a line\n"
    "    // --------------------------------------------------\n"
    "    std::cout << \"-- appending --\\n\";\n"
    "\n"
    "    auto fp3 = fopen(FILE_PATH, \"a\");\n"
    "    if (fp3) {\n"
    "        fprintf(fp3, \"line 5: appended at %d\\n\", millis());\n"
    "        fclose(fp3);\n"
    "        std::cout << \"appended.\\n\";\n"
    "    }\n"
    "\n"
    "    return 0;\n"
    "}\n";

// ------------------------------------------------------------
//  Flip this to 1 to print the preprocessed script over serial.
// ------------------------------------------------------------
#define DUMP_PREPROCESSED 0

// ------------------------------------------------------------
//  setup()
// ------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    delay(1000);

    printf("\n=== CMScript FileIO Example ===\n");

    // ---- Mount filesystem ----
    if (!MOUNT()) {
        printf("filesystem mount failed!\n");
#if defined(USE_LITTLEFS)
        printf("hint: run  Tools -> ESP32 Sketch Data Upload  once\n");
#endif
        return;
    }
    printf("filesystem mounted, prefix = %s\n", FS_PREFIX);

    // ---- Init engine ----
    CMScript_Init();
    CDM_RegisterHostFunc("__print_str",   serial_print);
    CDM_RegisterHostFunc("__print_int",   serial_print);
    CDM_RegisterHostFunc("__print_float", serial_print);

    // ---- Preprocess ----
    char* pre = CDM_Preprocess(src);
    if (!pre) { printf("preprocess failed\n"); return; }

#if DUMP_PREPROCESSED
    printf("---- preprocessed ----\n%s\n---- end ----\n", pre);
#endif

    // ---- Parse ----
    ASTNode_t* root = CDM_Parse(pre);
    free(pre);
    if (!root) { printf("parse failed\n"); return; }

    int errs = CDM_GetErrorCount();
    if (errs > 0) {
        for (int i = 0; i < errs; i++) {
            const CDM_Error_t* e = CDM_GetError(i);
            printf("parse error line %d: %s\n", e->line, e->message);
        }
        CDM_FreeAST(root);
        return;
    }

    // ---- Execute ----
    int rc = CDM_Execute(root);
    printf("main returned %d\n", rc);

    // ---- Cleanup ----
    CDM_FreeAST(root);

    // Optional: unmount filesystem
    // UMOUNT();

    printf("=== done ===\n");
}

void loop() {
    delay(1000);
}