
# Alpha Compiler

## Configuration Instructions

1. Create build directory (from root project directory):
```bash
mkdir build
cd build
cmake <OPTIONS> ..
```

## CMake Options

- **DEBUG_MODE**  
  Enables debug symbols (`-g`) and additional sanity checks. Sanity checks and debugging code are conditionally compiled within `#ifdef DEBUG_MODE` guards and `DEBUG_SMART_ASSERT` macros.

- **OPTIMIZED_MODE**  
  Enables compiler optimizations (`-O3`) and disables parser tracing. (The option `--show-parser-trace` becomes unavailable.)

- **ENABLE_IWYU**  
  Enables the Include-What-You-Use tool, ensuring minimal and correct inclusion of headers, thus reducing compilation time and improving code maintainability.

## Building the Project

Within the `build` directory, run:
```bash
make
```

### Additional make targets

- **Clean generated files:**  
```bash
make clean
```

- **Run specific validator test:**  
```bash
make run_validator_NAME
```

- **Run all validator tests:**  
```bash
make run_all_validators
```

Each test outputs `TEST_PASSED` or `TEST_FAILED`.

## Running `alpha_driver.out`

Executable options:

- `--input-file <file>`  
  **(Required)** Specifies the input alpha source file to parse.

- `--export-symbol-table`  
  Exports the compiler's symbol table to a CSV file named `<source_filename>.st.csv`.

- `--export-compile-errors`  
  Exports compiler errors to a CSV file named `<source_filename>.error.csv`.

- `--show-symbol-table`  
  Pretty-prints the symbol table on the console.

- `--show-parser-trace` *(Unavailable in OPTIMIZED_MODE)*  
  Prints parser grammar rules as they are matched.

- `--no-show-errors`  
  Disables error display, primarily for automated testing.

**Note:**  
Autocompletion is supported via a custom bash script. Press `TAB` to autocomplete available options.

## Technical Overview

- Uses Python scripts to inject tracing into semantic actions, reducing verbosity in Bison grammar files (`.y`).

- Employs Python scripts to run comprehensive validation tests (stored in `tests/` directory) verifying symbol tables and error outputs. Tests currently run post-compilation via `make`. Future goal: migrate tests fully to Python scripts.

- Semantic actions are decoupled from grammar rules, defined separately in `alpha_semantic_actions.hpp/.cpp`. This enhances grammar readability and simplifies error handling. Naming format: `LHS__RHS1_RHS2_..._RHSN`.

- Includes a custom command-line argument parser ("Arguinator"), inspired by Python’s `argparse`, providing structured CLI argument handling.

## Python Availability

If Python is unavailable or if the examiner demands immediate removal of Python scripts, the compiler project remains fully functional:

- Removing Python (`.py`) files disables parser tracing and automatic validator execution, which are non-essential runtime features.
- Core compiler features (compilation, symbol table generation, error reporting) remain fully operational.

## Format Adapter

To maintain compatibility across environments, the project uses the following format adapter:

```cpp
#ifndef FORMAT_ADAPTER_HPP
#define FORMAT_ADAPTER_HPP

#ifdef STD_FORMAT_SUPPORTED
#include <format>
namespace fmt_ns = std;
#else
#define FMT_HEADER_ONLY
#include "third_party/fmt/format.h"
namespace fmt_ns = fmt;
#endif // STD_FORMAT_SUPPORTED

#endif // FORMAT_ADAPTER_HPP
```

Initially, the developer's machine supported `std::format`, but the university’s Debian machines (gcc 10.2–12.4, clang 13) did not. A minimal, cherry-picked subset of the `fmt` library (version 11) is therefore included in `third_party/`. No external installation is required.
