// ============================================================
//  CMScript - Basic Example
// ------------------------------------------------------------
//  This example shows the minimal setup needed to:
//    1. Initialize the CMScript engine
//    2. Register a print callback (so the script can output text)
//    3. Preprocess a hardcoded script string
//    4. Parse it into an AST
//    5. Execute it
//    6. Free the AST
//
//  After you understand this file, look at Button.ino and
//  FileIO.ino for more advanced usage.
// ============================================================

#include <Arduino.h>
#include <CMScript.h>

// ------------------------------------------------------------
//  Print callback
// ------------------------------------------------------------
//  The engine does NOT know how to print. It calls back into
//  the host (this sketch) whenever the script does
//  `std::cout << ...`. We register three names:
//
//      __print_str    - for strings and chars
//      __print_int    - for integers
//      __print_float  - for floats
//
//  All three arrive with argc >= 1 and argv[0] holding the text
//  to print. Here we just forward to Serial.print().
// ------------------------------------------------------------
static int serial_print(int argc, const char** argv) {
    if (argc >= 1) Serial.print(argv[0]);
    return 0;
}

// ------------------------------------------------------------
//  The script itself
// ------------------------------------------------------------
//  Stored as a C++11 raw string literal R"CDM(...)CDM".
//
//  Why "CDM" as the delimiter? Because script code often
//  contains quotes and newlines. With a raw string literal:
//
//    - newlines are real newlines, not "\n"
//    - quotes are real quotes, not \"
//
//  The only rule: the sequence )CDM" must not appear inside
//  the script. If you ever need )" in a script, change the
//  delimiter to something else, e.g. R"CODE(...)CODE".
// ------------------------------------------------------------
static const char* src = R"CDM(int main() {
    // A simple computation and output
    std::cout << "hello CMScript\n";

    int a = 3;
    int b = 4;
    std::cout << "a + b = " << (a + b) << "\n";

    int c = a * b;
    std::cout << "a * b = " << c << "\n";

    // Loop from 0 to 4
    for (int i = 0; i < 5; i++) {
        std::cout << "i = " << i << "\n";
    }

    // Call a user-defined function
    int sq = square(5);
    std::cout << "square(5) = " << sq << "\n";

    return 0;
}

// User function defined outside main(). The engine collects
// all top-level functions before running main().
int square(int x) {
    return x * x;
}
)CDM";

// ------------------------------------------------------------
//  setup() - runs once at boot
// ------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    delay(1000);                           // wait for USB serial

    Serial.print("\n=== CMScript Basic Example ===\n");

    // ---- Step 1: initialize the engine ----
    //  This allocates internal tables (symbol table, host
    //  function table, struct definition table, ...) and
    //  registers all ArduinoBridge and std library functions.
    //
    //  It is safe to call this more than once; the second call
    //  is a no-op.
    CMScript_Init();

    // ---- Step 2: register our print callbacks ----
    //  Without these, `std::cout << ...` in the script would
    //  produce no visible output.
    CDM_RegisterHostFunc("__print_str",   serial_print);
    CDM_RegisterHostFunc("__print_int",   serial_print);
    CDM_RegisterHostFunc("__print_float", serial_print);

    // ---- Step 3: preprocess ----
    //  Expands #define / #ifdef / #include, strips comments in
    //  strings safely, and returns a new malloc'd buffer.
    char* pre = CDM_Preprocess(src);
    if (!pre) {
        Serial.print("preprocess failed\n");
        return;
    }

    // ---- Step 4: parse ----
    //  Builds an AST. On error, returns a non-NULL tree but
    //  CDM_GetErrorCount() > 0.
    ASTNode_t* root = CDM_Parse(pre);
    free(pre);                             // we own this buffer
    pre = NULL;

    if (!root) {
        Serial.print("parse failed\n");
        return;
    }

    int errs = CDM_GetErrorCount();
    if (errs > 0) {
        for (int i = 0; i < errs; i++) {
            const CDM_Error_t* e = CDM_GetError(i);
            char res[128];
            snprintf(res, sizeof(res),
                     "Error at line %d: %s\n", e->line, e->message);
            Serial.print(res);
        }
        CDM_FreeAST(root);
        return;
    }

    // ---- Step 5: execute ----
    //  Runs main() and prints timing + return value.
    int rc = CDM_Execute(root);

    // ---- Step 6: free ----
    //  Always free the AST after you're done with it.
    CDM_FreeAST(root);
    root = NULL;

    Serial.print("=== done ===\n");
}

// ------------------------------------------------------------
//  loop() - does nothing here
// ------------------------------------------------------------
void loop() {
    delay(1000);
}