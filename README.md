# CMScript

**C syntax. 30KB flash. Any MCU.**

A tiny C-subset scripting engine for microcontrollers. Write scripts that run
at runtime - change behavior without reflashing the firmware.

---

## Table of Contents

- [Why CMScript?](#why-cmscript)
- [Features](#features)
- [Quick Start](#quick-start)
- [Installation](#installation)
- [Usage](#usage)
- [Script Syntax](#script-syntax)
- [Standard Library](#standard-library)
- [Examples](#examples)
- [Host API Reference](#host-api-reference)
- [Performance](#performance)
- [Limitations](#limitations)
- [Supported Platforms](#supported-platforms)
- [Contributing](#contributing)
- [License](#license)

---

## Why CMScript?

You already know C. Why learn Lua to change your firmware?

| Lua quirk                    | CMScript              |
|------------------------------|-----------------------|
| `local x = 1`                  | `int x = 1`             |
| `a .. b  (string concat)`      | `a + b`                 |
| `then ... end`                 | `{ ... }`               |
| `elseif`                       | `else if`               |
| `t:method()`                   | `t.method()`            |
| arrays start at 1            | arrays start at 0     |
| nil everywhere               | explicit types        |
| no ++, +=, switch            | all present           |

If you write C, you already know CMScript.

---

## Features

- Preprocessor
    - #define, #undef, #ifdef, #ifndef, #elif, #else, #endif
    - #include with custom resolver callback
    - #pragma once (via include tracking)

- Language
    - Variables, functions, recursion
    - if / else / while / do-while / for / switch / break / continue
    - Ternary operator: cond ? a : b
    - Full C operator set including +=, -=, <<=, ++, --, bitwise ops
    - Type casts: (int), (float)
    - sizeof(expr) and sizeof(type)

- Data types
    - int (64-bit internally), float, string, bool
    - Fixed-size arrays (multi-dimensional)
    - struct with fields
    - pointers, references

- Standard library
    - Math: sqrt, sin, cos, tan, pow, log, log2, abs, min, max, ...
    - String: length, substr, find, replace, trim, c_str, at, ...
    - Global string functions: strlen, strcmp, strcpy, strcat, strncpy, strncmp, strchr
    - Formatted I/O: sprintf, snprintf, printf
    - File I/O: fopen, fclose, fprintf, fputs, fgets, fread, fwrite, feof
    - Number conversion: atoi, atof, to_string

- Host bridge
    - Register any native C function with one line
    - Supports both int-argc/char-argv style and CDM_Value_t style
    - Reference semantics for pass-by-reference

- Developer experience
    - Line numbers on every runtime error
    - Execution time printed after each run
    - AST cache for faster repeated execution

---

## Quick Start

```cpp
    #include <CMScript.h>

    static const char* src =
        "int main() {\n"
        "    std::cout << \"hello CMScript\\n\";\n"
        "    int a = 3;\n"
        "    int b = 4;\n"
        "    std::cout << \"a + b = \" << (a + b) << \"\\n\";\n"
        "    return 0;\n"
        "}\n";

    static int serial_print(int argc, const char** argv) {
        if (argc >= 1) printf("%s", argv[0]);
        return 0;
    }

    void setup() {
        Serial.begin(115200);
        delay(1000);

        CMScript_Init();
        CDM_RegisterHostFunc("__print_str",   serial_print);
        CDM_RegisterHostFunc("__print_int",   serial_print);
        CDM_RegisterHostFunc("__print_float", serial_print);

        char* pre = CDM_Preprocess(src);
        ASTNode_t* root = CDM_Parse(pre);
        free(pre);

        if (root && CDM_GetErrorCount() == 0) {
            CDM_Execute(root);
        }
        if (root) CDM_FreeAST(root);
    }

    void loop() {}
```

Expected serial output:
```
    hello CMScript
    a + b = 7
    exit after 0.001s returned 0
```

---

## Installation

### Arduino IDE (Library Manager)

1. Open Arduino IDE
2. Sketch -> Include Library -> Manage Libraries
3. Search for "CMScript"
4. Click Install

### Arduino IDE (Manual)

1. Download the latest release from GitHub
2. Extract to Documents/Arduino/libraries/KernelCDM
3. Restart Arduino IDE
4. Sketch -> Include Library -> CMScript should now appear

### PlatformIO

Add to platformio.ini:

```ini
    lib_deps =
        yourname/CMScript@^1.0.0
```

Or if installing from local folder:

```ini
    lib_extra_dirs =
        path/to/your/libraries
```

---

## Usage

### Basic workflow

The engine works in four stages: preprocess, parse, execute, free.

```cpp
    // 1. Preprocess (expand #define, #include, ...)
    char* pre = CDM_Preprocess(src);
    if (!pre) return;

    // 2. Parse (build AST)
    ASTNode_t* root = CDM_Parse(pre);
    free(pre);            // preprocess output is yours to free
    if (!root) return;

    // 3. Check for parse errors
    int errs = CDM_GetErrorCount();
    for (int i = 0; i < errs; i++) {
        const CDM_Error_t* e = CDM_GetError(i);
        printf("line %d: %s\n", e->line, e->message);
    }
    if (errs > 0) {
        CDM_FreeAST(root);
        return;
    }

    // 4. Execute
    int rc = CDM_Execute(root);

    // 5. Free
    CDM_FreeAST(root);
```

### Registering host functions

Two ways:

Style 1 - string arguments (simple):

```cpp
    static int my_print(int argc, const char** argv) {
        if (argc >= 1) printf("%s", argv[0]);
        return 0;
    }

    CDM_RegisterHostFunc("my_print", my_print);
```

Style 2 - typed values (for passing by reference):

```cpp
    static CDM_Value_t my_read(int argc, CDM_Value_t* argv) {
        int pin = (int)argv[0].v.i;
        int val = digitalRead(pin);
        return CDM_MakePtr(&val, 0);
    }

    CDM_RegisterHostFuncEx("my_read", my_read);
```

Note: if a script argument is an lvalue (variable, array element, struct
field), it is passed as a REF. Use CDM_IsRef and CDM_AssignRef to write back.

### Registering an include resolver

By default, #include directives are ignored with a warning. To support them,
register a callback that returns the file content:

```cpp
    static char* my_resolver(const char* filename) {
        // Return a malloc'd buffer, or NULL if not found
        char path[64];
        snprintf(path, sizeof(path), "/littlefs/%s", filename);
        File f = LittleFS.open(path, "r");
        if (!f) return NULL;
        size_t sz = f.size();
        char* buf = (char*)malloc(sz + 1);
        f.read((uint8_t*)buf, sz);
        buf[sz] = 0;
        f.close();
        return buf;
    }

    CDM_SetIncludeResolver(my_resolver);
```

### Error handling

Two kinds of errors: parse errors and runtime errors.

Parse errors - call CDM_GetErrorCount / CDM_GetError after CDM_Parse:

```cpp
    int errs = CDM_GetErrorCount();
    for (int i = 0; i < errs; i++) {
        const CDM_Error_t* e = CDM_GetError(i);
        printf("parse error line %d: %s\n", e->line, e->message);
    }
```

Runtime errors - the engine prints them to stdout and continues with a
default value. Every error includes a line number:

    [CDM] line 12: division by zero
    [CDM] line 88: array index [0]=10 out of range [0,8)

To stop on first runtime error, call CDM_GetErrorCount after CDM_Execute.

---

## Script Syntax

### Preprocessor

```cpp
    #define PI 3.14159
    #define LED_PIN 2

    #ifdef DEBUG
        std::cout << "debug mode\n";
    #endif

    #ifdef FEATURE_A
        ...
    #elif defined(FEATURE_B)
        ...
    #else
        ...
    #endif

    #include "config.h"
    #include <utils.h>

    #pragma once
```

### Types

```cpp
    int a = 10;
    float f = 3.14;
    std::string s = "hello";
    bool flag = true;

    int arr[10];
    int matrix[3][4];

    struct Point { int x; int y; };
    Point p;
    p.x = 10;
    p.y = 20;

    int* ptr = new int;
    *ptr = 42;
    delete ptr;

    int* arr2 = new int[100];
    delete[] arr2;
```

### Statements

```cpp
    if (cond) { ... } else { ... }

    while (cond) { ... }

    do { ... } while (cond);

    for (int i = 0; i < 10; i++) { ... }

    for (int x : arr) { ... }        // range-for

    switch (val) {
        case 0: ...; break;
        case 1:
        case 2: ...; break;
        default: ...; break;
    }

    break;
    continue;
    return value;
```

### Operators

```
Arithmetic:     + - * / %
Comparison:     == != < > <= >=
Logical:        && || !
Bitwise:        & | ^ ~ << >>
Assignment:     = += -= *= /= %= &= |= ^= <<= >>=
Increment:      ++ --
Ternary:        cond ? a : b
Cast:           (int)x, (float)y
Sizeof:         sizeof(int), sizeof(arr), sizeof(arr[0])
```

### String methods

```
    std::string s = "hello world";

    s.length()              -> 11
    s.size()                -> 11 (alias)
    s.empty()               -> 0
    s.substr(0, 5)          -> "hello"
    s.find("world")         -> 6
    s.append("!")           -> "hello world!"
    s.replace(0, 5, "HI")   -> "HI world"
    s.trim()                -> trims whitespace
    s.c_str()               -> C string copy
    s.at(0)                 -> "h"
```

---

## Standard Library

### Math

```
sqrt, sin, cos, tan, asin, acos, atan, atan2,
floor, ceil, round, fabs, fmod, pow, exp,
log, log10, log2, abs, min, max, rand, srand
```

### String

```
strlen, strcmp, strcpy, strcat, strncpy, strncmp, strchr
```

### Formatting

```cpp
    sprintf(buf, "x = %d, y = %d", x, y);
    snprintf(buf, 16, "%s", long_string);
    printf("hello %s, n = %d\n", "world", 42);
```

Supported format specifiers:

```
    %d %i    signed integer
    %u       unsigned integer
    %x %X    hexadecimal
    %o       octal
    %f %e %g floating point
    %s       string
    %c       character
    %p       pointer
    %%       literal percent
```

Note: width and precision (.2f, %5d) are currently ignored.

### File I/O

```cpp
    auto fp = fopen("/littlefs/log.txt", "w");
    if (fp == 0) {
        std::cout << "open failed\n";
        return 1;
    }
    fprintf(fp, "boot count = %d\n", 42);
    fputs("done\n", fp);
    fclose(fp);

    // Read back
    auto fp2 = fopen("/littlefs/log.txt", "r");
    std::string line = "";
    while (!feof(fp2)) {
        line = fgets(line, 128, fp2);
        if (line.length() > 0) {
            std::cout << line << "\n";
        }
    }
    fclose(fp2);
```

Supported modes: "r", "w", "a", "rb", "wb", "ab", "r+", "w+", "a+"

Note: on ESP32, the path must include the mount point prefix:

```
  /littlefs/foo.txt  (for LittleFS)
  /spiffs/foo.txt    (for SPIFFS)
  /sd/foo.txt        (for SD card)
```

### Number conversion

```
atoi, atof, to_string
```

---

## Examples

See the examples/ folder:

- examples/Basic      - minimal setup and hello world
- examples/Button     - read GPIO and print on state change
- examples/FileIO     - write, read, and append to a file

To open: File -> Examples -> CMScript -> (example name)

---

## Host API Reference

### Initialization

```cpp
    void CMScript_Init(void);
```

Call once in setup(). Registers ArduinoBridge and std library functions.
Safe to call multiple times.

### Preprocessing

```cpp
    char* CDM_Preprocess(const char* src);
```

Returns a malloc'd buffer. Caller must free().
Returns NULL on allocation failure.

```cpp
    void CDM_SetIncludeResolver(CDM_IncludeResolver_t fn);

    typedef char* (*CDM_IncludeResolver_t)(const char* filename);
```

Callback must return a malloc'd buffer (engine will free it), or NULL
if the file is not found.

```cpp
    void CDM_ResetIncludes(void);
```

Clears the list of already-included files.

### Parsing

```cpp

    ASTNode_t* CDM_Parse(const char* src);

Returns an AST. Never returns NULL on syntax error, but sets error count.

    int CDM_GetErrorCount(void);
    const CDM_Error_t* CDM_GetError(int idx);

    typedef struct {
        int line;
        int col;
        char message[80];
    } CDM_Error_t;

    void CDM_ClearErrors(void);
```

### Execution

```cpp
    int CDM_Execute(ASTNode_t* root);
```

Runs main() and returns its return value.

### Cleanup

```cpp
    void CDM_FreeAST(ASTNode_t* root);
```

Frees the entire AST. Always call after execution.

### Host functions

```cpp

    void CDM_RegisterHostFunc(const char* name, CDM_HostFunc_t fn);
    void CDM_RegisterHostFuncEx(const char* name, CDM_HostFuncEx_t fn);
    void CDM_RegisterHostFuncFloat(const char* name, CDM_HostFunc_t fn);

    typedef int (*CDM_HostFunc_t)(int argc, const char** argv);
    typedef CDM_Value_t (*CDM_HostFuncEx_t)(int argc, CDM_Value_t* argv);
```

Use HostFunc for simple string-based calls.
Use HostFuncEx for typed values and pass-by-reference.
Use HostFuncFloat to mark the return value as float.

### Value helpers

```cpp

    CDM_Value_t CDM_MakeString(const char* s);
    CDM_Value_t CDM_MakeStringOwned(char* s);
    CDM_Value_t CDM_MakePtr(void* p, int owned);

    int          CDM_IsRef(CDM_Value_t v);
    CDM_Value_t  CDM_Deref(CDM_Value_t v);
    CDM_Value_t* CDM_RefPtr(CDM_Value_t v);
    void         CDM_AssignRef(CDM_Value_t ref, CDM_Value_t value);
```

### Value struct

```cpp

    typedef struct {
        CDM_ValueType_t type;      // INT, FLOAT, PTR, STRUCT, ARRAY, STRING, REF
        int             struct_id;
        int             array_len;
        int             array_dim;
        int             array_sizes[4];
        CDM_ValueType_t array_elem_type;
        int             array_elem_struct_id;
        int             owned;
        int             from_new;
        union {
            int64_t i;
            double  f;
            void*   p;
        } v;
    } CDM_Value_t;
```

---

## Performance

Measured on ESP32-S3 @ 240MHz, 240KB free heap:

    Simple loop (int increment):     ~45,000 iterations / second
    Boot to first script:            under 100 ms
    Preprocess + parse 100-line:     under 5 ms
    Memory (steady state):           about 65 KB heap

On ESP32 classic @ 240MHz, expect about 30 percent slower.
On ESP8266 @ 160MHz, expect about 3x slower.

The interpreter is a tree-walker, not a bytecode VM. This is intentional:
simplicity and small code size are more important than raw speed for the
target use case (scripts that run a few times per second).

---

## Limitations

Known limitations (not bugs):

- No classes, templates, inheritance
- No exceptions (use error codes)
- No function overloading
- No goto / labels
- No #elif with complex conditions (only defined(NAME) or NAME)
- No function-like macros like #define MAX(a,b) ...
- Width and precision in printf/sprintf are ignored (.2f works but the .2
  has no effect - output uses %g formatting)
- AVR (Mega2560) support is experimental - 8KB SRAM is tight

---

## Supported Platforms

Fully tested:

- ESP32 (all variants including S2, S3, C3)
- ESP8266
- RP2040 (Raspberry Pi Pico)

Partially tested:

- SAMD21 / SAMD51 (Arduino Zero, MKR series)
- STM32 (via Arduino STM32 core)

Experimental:

- AVR (Arduino Uno, Mega2560) - needs reduced limits, see Wiki

Not supported:

- Boards with less than 4KB SRAM

---

## Contributing

Pull requests welcome. Please:

1. Keep the code C89-compatible (no C99 VLAs in hot paths)
2. Test on at least one ESP32 board before submitting
3. Update the README if you add a public API
4. Add an example if you add a user-visible feature

Bug reports: please include
- Board and core version
- Minimal script that reproduces the issue
- Full serial output including the exit-after line

---

## License

Apache 2.0 - see LICENSE file for details.
