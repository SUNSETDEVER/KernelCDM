#include "CMScript.h"

void CMScript_Init(void) {
    CDM_ConfigStack(512);
    CDM_ArduinoBridge_Init();
    CDM_Std_Init();
}