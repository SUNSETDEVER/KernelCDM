# CMScript

**C syntax. 30KB flash. Any MCU.**

A tiny C-subset scripting engine for microcontrollers. Write scripts that run at
runtime — change behavior without reflashing the firmware.

## Why CMScript?

You already know C. Why learn Lua to change your firmware?

| Lua's quirks | CMScript |
|--------------|----------|
| `local x = 1` | `int x = 1` |
| `a .. b` for string concat | `a + b` |
| `then ... end` | `{ ... }` |
| `elseif` | `else if` |
| `t:method()` | `t.method()` |
| Arrays start at 1 | Arrays start at 0 |
| `nil` everywhere | Explicit types |

If you write C, you already know CMScript.

## Features

- **Preprocessor**: `#define`, `#undef`, `#ifdef`, `#ifndef`, `#elif`, `#include`, `#pragma once`
- **Syntax**: if / else / while / for / do-while / switch / break / continue / ternary
- **Types**: int, float, string, array, struct, pointer, reference
- **Operators**: full C set including `+=` `<<=` `++` `--` and bitwise ops
- **std library**: math, `std::string` methods, `sprintf`, `printf`, `fopen` / `fprintf` / `fgets`
- **Host bridge**: register any native function with one line
- **Error reporting**: line numbers on every runtime error

## Quick start

```cpp
#include <CMScript.h>

static const char* src = R"(
int main() {
    std::cout << "hello CMScript\n";
    return 0;
}
)";

static int serial_print(int argc, const char** argv) {
    if (argc >= 1) printf("%s", argv[0]);
    return 0;
}

void setup() {
    Serial.begin(115200);
    CMScript_Init();
    CDM_RegisterHostFunc("__print_str", serial_print);
    CDM_RegisterHostFunc("__print_int", serial_print);
    CDM_RegisterHostFunc("__print_float", serial_print);

    char* pre = CDM_Preprocess(src);
    ASTNode_t* root = CDM_Parse(pre);
    free(pre);
    CDM_Execute(root);
    CDM_FreeAST(root);
}

void loop() {}
```

## Performance

On ESP32-S3 @ 240MHz:

- ~15,000 simple loop iterations / second
- Boot-to-first-script: < 100ms
- Preprocessing + parsing a 100-line script: < 5ms

## License

Apache v2.0 — see [LICENSE](LICENSE).
