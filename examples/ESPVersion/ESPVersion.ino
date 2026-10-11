#include <Arduino.h>
#include <CMScript.h>

#define INTERP_STACK_WORDS 8192
#define INTERP_PRIORITY 1
#define INTERP_CORE 1

static const char* src =
  "int main() {\n"
  "    std::cout << \"hello\\n\"\n"
  "    return 0;\n"
  "}\n";

static int serial_print(int argc, const char** argv) {
  if (argc >= 1) printf("%s", argv[0]);
  return 0;
}

static void interpreterTask(void* pv) {
  CMScript_Init();
  CDM_RegisterHostFunc("__print_str", serial_print);
  CDM_RegisterHostFunc("__print_int", serial_print);
  CDM_RegisterHostFunc("__print_float", serial_print);

  char* pre = CDM_Preprocess(src);
  ASTNode_t* root = CDM_Parse(pre);
  free(pre);

  int errs = CDM_GetErrorCount();
  if (errs > 0) {
    for (int i = 0; i < errs; i++) {
      const CDM_Error_t* e = CDM_GetError(i);
      printf("Error at line %d: %s\n", e->line, e->message);
    }
    CDM_FreeAST(root);
  } else {
    if (root) {
      CDM_Execute(root);
    }
    if (root) CDM_FreeAST(root);
  }

  // Optional: print remaining stack to tune the size
  UBaseType_t remain = uxTaskGetStackHighWaterMark(NULL);
  printf("stack high-water: %u words (%u bytes)\n",
         (unsigned)remain,
         (unsigned)(remain * sizeof(StackType_t)));

  vTaskDelete(NULL);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  xTaskCreatePinnedToCore(
    interpreterTask,
    "Interp",
    INTERP_STACK_WORDS,  // NOTE: unit is WORDS on ESP32 (1 word = 4 bytes)
    NULL,
    INTERP_PRIORITY,
    NULL,
    INTERP_CORE);
}

void loop() {
  delay(1000);
}