// ============================================================
//  CMScript - Button Example
// ------------------------------------------------------------
//  Wiring:
//    Button between GPIO4 and GND.
//    Internal pull-up is used, so no external resistor.
//
//    GPIO4 ---[ button ]--- GND
//
//  Not pressed  -> GPIO4 reads 1
//  Pressed      -> GPIO4 reads 0
//
//  Avoid GPIO0 (boot), GPIO6-11 (flash), GPIO34-39 (no pull-up).
// ============================================================

#include <Arduino.h>
#include <CMScript.h>

// ------------------------------------------------------------
//  Configuration (injected into the script via STR macro)
// ------------------------------------------------------------
#define BUTTON_PIN      4
#define DEBOUNCE_MS     30
#define RUNTIME_MS      15000
#define POLL_INTERVAL   10

// ------------------------------------------------------------
//  Stringify helpers - MUST appear before first use
// ------------------------------------------------------------
#define STR_HELPER(x) #x
#define STR(x)        STR_HELPER(x)

// ------------------------------------------------------------
//  Script source
// ------------------------------------------------------------
static const char* src =
    "#define BUTTON_PIN     " STR(BUTTON_PIN)     "\n"
    "#define DEBOUNCE_MS    " STR(DEBOUNCE_MS)    "\n"
    "#define RUNTIME_MS     " STR(RUNTIME_MS)     "\n"
    "#define POLL_INTERVAL  " STR(POLL_INTERVAL)  "\n"
    "\n"
    "int main() {\n"
    "    std::cout << \"Button test start\\n\";\n"
    "    std::cout << \"Press button on GPIO\" << BUTTON_PIN << \"\\n\";\n"
    "\n"
    "    pinMode(BUTTON_PIN, INPUT_PULLUP);\n"
    "\n"
    "    unsigned int start     = millis();\n"
    "    unsigned int lastEdge  = 0;\n"
    "    int          lastState = digitalRead(BUTTON_PIN);\n"
    "    int          presses   = 0;\n"
    "    int          releases  = 0;\n"
    "\n"
    "    std::cout << \"initial = \" << lastState << \"\\n\";\n"
    "\n"
    "    while (millis() - start < RUNTIME_MS) {\n"
    "        int now = digitalRead(BUTTON_PIN);\n"
    "\n"
    "        if (now != lastState) {\n"
    "            unsigned int t = millis();\n"
    "            if (t - lastEdge >= DEBOUNCE_MS) {\n"
    "                lastEdge  = t;\n"
    "                lastState = now;\n"
    "\n"
    "                if (now == 0) {\n"
    "                    presses++;\n"
    "                    std::cout << \"press   #\" << presses << \"\\n\";\n"
    "                } else {\n"
    "                    releases++;\n"
    "                    std::cout << \"release #\" << releases << \"\\n\";\n"
    "                }\n"
    "            }\n"
    "        }\n"
    "        delay(POLL_INTERVAL);\n"
    "    }\n"
    "\n"
    "    std::cout << \"\\ntimeout\\n\";\n"
    "    std::cout << \"presses  = \" << presses  << \"\\n\";\n"
    "    std::cout << \"releases = \" << releases << \"\\n\";\n"
    "    return 0;\n"
    "}\n";

// ------------------------------------------------------------
//  Print callback
// ------------------------------------------------------------
static int serial_print(int argc, const char** argv) {
    if (argc >= 1) printf("%s", argv[0]);
    return 0;
}

// ------------------------------------------------------------
//  setup()
// ------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    delay(1000);

    printf("\n=== CMScript Button Example ===\n");
    printf("GPIO%d, %dms runtime\n", BUTTON_PIN, RUNTIME_MS);

    CMScript_Init();
    CDM_RegisterHostFunc("__print_str",   serial_print);
    CDM_RegisterHostFunc("__print_int",   serial_print);
    CDM_RegisterHostFunc("__print_float", serial_print);

    char* pre = CDM_Preprocess(src);
    if (!pre) { printf("preprocess failed\n"); return; }

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

    int rc = CDM_Execute(root);
    printf("main returned %d\n", rc);
    CDM_FreeAST(root);
    printf("=== done ===\n");
}

void loop() {
    delay(1000);
}