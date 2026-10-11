# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

### Planned
- Function-like macros in preprocessor: #define MAX(a,b) ...
- Complex #elif conditions (&&, ||, comparison)
- goto / labels
- String split() and join()
- I2C / SPI convenience wrappers
- Interactive REPL over serial
- Script hot-reload from filesystem

---

## [1.0.0] - 2026-10-11

First public release.

### Added

**Core engine**
- Preprocessor
  - #define and #undef (object-like macros only)
  - #ifdef / #ifndef / #elif / #else / #endif
  - #include with custom resolver callback
  - #pragma once via include tracking
- Lexer and recursive-descent parser
- Tree-walk interpreter with AST cache
- Line numbers on every parse and runtime error
- Execution time reported after each run

**Language**
- Variables with int, float, string, bool types
- Functions with parameters, recursion, return values
- Control flow: if / else / while / do-while / for / range-for / switch / break / continue
- Ternary operator: cond ? a : b
- Full C operator set:
  - Arithmetic: + - * / %
  - Comparison: == != < > <= >=
  - Logical: && || !
  - Bitwise: & | ^ ~ << >>
  - Assignment: = += -= *= /= %= &= |= ^= <<= >>=
  - Increment: ++ -- (prefix and postfix)
- Type casts: (int), (float)
- sizeof(expr) and sizeof(type)
- enum with explicit and implicit values
- struct with fields
- Multi-dimensional fixed-size arrays
- Range-for over arrays
- new / delete for scalar and array
- References and pointer semantics

**Standard library**
- Math: sqrt, sin, cos, tan, asin, acos, atan, atan2, floor, ceil, round, fabs, fmod, pow, exp, log, log10, log2, abs, min, max, rand, srand
- std::string class:
  - Constructors from string, int, float
  - Methods: length, size, empty, substr, find, append, replace, trim, c_str, at
- Global string functions: strlen, strcmp, strcpy, strcat, strncpy, strncmp, strchr
- Formatted output: sprintf, snprintf, printf
  - Supports %d %i %u %x %X %o %f %e %g %s %c %p %%
- File I/O: fopen, fclose, fprintf, fputs, fgets, fread, fwrite, feof
- Number conversion: atoi, atof, to_string

**Host bridge**
- CDM_RegisterHostFunc for simple string-based calls
- CDM_RegisterHostFuncEx for typed values and pass-by-reference
- CDM_RegisterHostFuncFloat for float return values
- Automatic REF passing for lvalues
- CDM_MakeString, CDM_MakeStringOwned, CDM_MakePtr
- CDM_IsRef, CDM_Deref, CDM_RefPtr, CDM_AssignRef

**Arduino bridge**
- pinMode, digitalWrite, digitalRead
- analogWrite, analogRead
- delay, delayMicroseconds, millis, micros
- map, constrain
- Serial input helpers: __read_int, __read_float

**Examples**
- Basic: minimal setup and hello world
- Button: read GPIO with debounce and state-change printing
- FileIO: write, read, and append to a file

**Documentation**
- README with quick start, API reference, and limitations
- library.properties for Arduino Library Manager
- keywords.txt for IDE syntax highlighting

### Known Limitations
- No classes, templates, or inheritance
- No exceptions (use error codes)
- No function overloading
- No goto / labels
- No function-like macros
- No complex #elif conditions (only defined(NAME) or NAME)
- Width and precision in printf/sprintf are ignored
- AVR support is experimental

---

## Types of Changes

- `Added` for new features
- `Changed` for changes in existing functionality
- `Deprecated` for soon-to-be removed features
- `Removed` for now removed features
- `Fixed` for any bug fixes
- `Security` for vulnerability fixes
