// ============================================================
//  CMScript - Performance Benchmark
// ------------------------------------------------------------
//  Measures raw interpreter speed with minimal host-call overhead.
//
//  Instead of calling millis() on every iteration (which is a
//  host-function call and dominates the measurement), we use an
//  outer loop that checks millis() only once per 1000 inner
//  iterations.
// ============================================================

#include <Arduino.h>
#include <CMScript.h>

#define INTERP_STACK_BYTES 24 * 1024
#define INTERP_PRIORITY 1
#define INTERP_CORE 1

// ------------------------------------------------------------
//  Script
// ------------------------------------------------------------
//  Three benchmarks:
//    1. Integer add   (i32Add += 1)
//    2. Float add     (floatAdd += 1.0f)
//    3. Multiply      (mulAdd *= 2)
//
//  Each runs for 1 second of wall-clock time, using an inner
//  loop of 1000 iterations between millis() checks.
// ------------------------------------------------------------
static const char* src = R"CDM(int main() {
    uint32_t start;
    uint32_t n;

    // ---- 1. integer add ----
    uint32_t i32Add = 0;
    start = millis();
    while (millis() - start < 1000) {
        for (n = 0; n < 1000; n++) {
            i32Add += 1;
        }
    }
    std::cout << i32Add << " int adds per second\n";

    // ---- 2. float add ----
    float floatAdd = 0.0f;
    start = millis();
    while (millis() - start < 1000) {
        for (n = 0; n < 1000; n++) {
            floatAdd += 1.0f;
        }
    }
    std::cout << (uint32_t)floatAdd << " float adds per second\n";

    // ---- 3. multiply ----
    uint32_t mulAdd = 1;
    uint32_t mulCount = 0;
    start = millis();
    while (millis() - start < 1000) {
        for (n = 0; n < 1000; n++) {
            mulAdd = mulAdd * 2 + 1;
            if (mulAdd > 1000000) mulAdd = 1;
            mulCount++;
        }
    }
    std::cout << mulCount << " mul ops per second\n";

    return 0;
}
)CDM";

// ------------------------------------------------------------
//  Print callback
// ------------------------------------------------------------
static int serial_print(int argc, const char** argv) {
  if (argc >= 1) printf("%s", argv[0]);
  return 0;
}

// ------------------------------------------------------------
//  Interpreter task
// ------------------------------------------------------------
static void interpreterTask(void* pv) {
  printf("\n=== CMScript Benchmark ===\n");

  CMScript_Init();
  CDM_RegisterHostFunc("__print_str", serial_print);
  CDM_RegisterHostFunc("__print_int", serial_print);
  CDM_RegisterHostFunc("__print_float", serial_print);

  // ---- Preprocess ----
  char* pre = CDM_Preprocess(src);
  if (!pre) {
    printf("preprocess failed\n");
    vTaskDelete(NULL);
    return;
  }

  // ---- Parse ----
  ASTNode_t* root = CDM_Parse(pre);
  free(pre);
  if (!root) {
    printf("parse failed\n");
    vTaskDelete(NULL);
    return;
  }

  int errs = CDM_GetErrorCount();
  if (errs > 0) {
    for (int i = 0; i < errs; i++) {
      const CDM_Error_t* e = CDM_GetError(i);
      printf("Error at line %d: %s\n", e->line, e->message);
    }
    CDM_FreeAST(root);
    vTaskDelete(NULL);
    return;
  }

  // ---- Execute ----
  CDM_Execute(root);
  CDM_FreeAST(root);

  UBaseType_t bytes = uxTaskGetStackHighWaterMark(NULL);
  printf("stack high-water: %u bytes, %u used\n", bytes, INTERP_STACK_BYTES - bytes);

  printf("=== done ===\n");
  vTaskDelete(NULL);
}

// ------------------------------------------------------------
//  Arduino entry
// ------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  xTaskCreatePinnedToCore(
    interpreterTask,
    "Interp",
    INTERP_STACK_BYTES,
    NULL,
    INTERP_PRIORITY,
    NULL,
    INTERP_CORE);
}

void loop() {
  delay(1000);
}