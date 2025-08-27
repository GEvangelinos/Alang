
# Alpha Compiler | Georgios Evangelinos 

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
  Enables compiler optimizations (`-O3`, `-march=native`, `(-flto)`) and disables parser tracing. (The option `--show-parser-trace` becomes inert.)

- **ENABLE_IWYU**  
  Enables the Include-What-You-Use tool, ensuring minimal and correct inclusion of headers, thus reducing compilation time and improving code maintainability.

- **HATE_PYTHON_MODE**  
  Disables all utilities that require Python3, like generating parser-trace and create the regression_check targets for automated testing. (The option `--show-parser-trace` becomes inert.)

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

- **Runs Alpha Compiler's testfiles to test for correct output:**  
```bash
make regression_check
```

- **Runs Alpha Compiler's testfiles to test for correct output and memory leaks :**  
```bash
make regression_check_full
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

- `--show-parser-trace` *(Unavailable in OPTIMIZED_MODE and HATE_PYTHON_MODE)*  
  Prints parser grammar rules as they are matched.

- `--no-show-errors`  
  Disables error display, primarily for automated testing.

**Note:**  
Autocompletion is supported via a custom bash script. Press `TAB` to autocomplete available options.

## Technical Overview

- Uses Python scripts to inject tracing into semantic actions, reducing verbosity in Bison grammar files (`.y`).

- Employs Python scripts to run comprehensive validation tests (stored in `tests/` directory) verifying symbol tables and error outputs and memory leaks. Tests run post-compilation via `python` scripts. Through two explicit make targets.
  - `regression_check` verifying the output of the Alpha Compiler.
  - `regression_check_full` Does all that `regression_check` does, plus it employs `Valgrind` to check for memory leaks (any kind).

- Semantic actions are decoupled from grammar rules, defined separately in `alpha_semantic_actions.hpp/.cpp`. This enhances grammar readability and simplifies error handling. Naming format: `LHS__RHS1_RHS2_..._RHSN`.

- Includes a custom command-line argument parser ("Arguinator"), inspired by Python’s `argparse`, providing structured CLI argument handling.

## Rich Diagnostics

The compiler features **rich diagnostic messages**, loosely inspired by GCC's error output style. Errors are annotated with file name, line, and column numbers, along with contextual notes when relevant.

**Example output:**

```bash
[stygian@sepermeru build]$ ./executables/alpha_driver.out --input-file ../.tests/error/Error1.asc
Error1.asc:6:16: error: variable `f` is not accessible in function `g`.
       6 |                f=5;
         |                ^
Error1.asc:5:17: note: function `g` declared here
       5 |        function g(){
         |                 ^
Error1.asc:4:14: note: variable `f` declared here
       4 |        local f=10;
         |              ^
Error1.asc:7:23: error: variable `f` is not accessible in function `g`.
       7 |                return f();
         |                       ^
Error1.asc:5:17: note: function `g` declared here
       5 |        function g(){
         |                 ^
Error1.asc:4:14: note: variable `f` declared here
       4 |        local f=10;
         |              ^
[stygian@sepermeru build]$
```
This style aims to help users immediately locate, understand, and resolve errors in their code.

## Python Availability

If Python is unavailable or if the examiner demands immediate deactivation of Python scripts, the compiler project remains fully functional:

- Deactivating Python (`.py`) by defining `HATE_PYTHON_MODE` in CMAKE files disables parser tracing and automatic validator execution, which are non-essential runtime features.
```base
cmake -DHATE_PYTHON_MODE=ON <PROJECT_ROOT_DIR>
```
- Core compiler features (compilation, symbol table generation, error reporting) remain fully operational.

## Format Adapter

To maintain compatibility across environments, the project uses the following format adapter:

```cpp
#ifndef FORMAT_ADAPTER_HPP
#define FORMAT_ADAPTER_HPP

#ifdef STD_FORMAT_SUPPORTED
#include <format>
namespace FMT = std;
#else
#define FMT_HEADER_ONLY
#include "third_party/fmt/format.h"
namespace FMT = fmt;
#endif // STD_FORMAT_SUPPORTED

#endif // FORMAT_ADAPTER_HPP
```

Initially, the developer's machine supported `std::format`, but the university’s Debian machines (gcc 10.2–12.4, clang 11) did not. A minimal, cherry-picked subset of the `fmt` library (version 11) is therefore included in `third_party/`. No external installation is required.



\newpage
=====================================================================
Deviation: Plain Assignment
=====================================================================

Input:
```
x = 5;
```

Output:
```
assign   x    5
```

Explanation:
In lecture notes, every assignment also emits a temporary
(e.g. `_t0 := x`) because assignments are expressions.  
In my project, when the assignment appears only as a statement,
I omit the extra temp. The expression’s value is unused in this
context, so leaving out `_t0` avoids dead quads and keeps the IR clean.

\newpage
=====================================================================
Deviation: Assignment Inside Call
=====================================================================

Input:
```
a(x=1, x=2, x=3);
```

Output (params loaded right-to-left, per project spec):
```
quad#   opcode   result   arg1   arg2   label   line
----------------------------------------------------
1       assign   x        3                         2
2       assign   _t2      x                         2
3       assign   x        2                         2
4       assign   _t1      x                         2
5       assign   x        1                         2
6       assign   _t0      x                         2
7       param             _t0                       2
8       param             _t1                       2
9       param             _t2                       2
10      call              a                         2
11      getretval _t3                               2
```

Explanation:
Inside calls, assignments must yield values in addition to updating `x`.
Otherwise, all arguments would collapse to the final value of `x`
(`f(3,3,3)`). Temporaries (`_t0.._t2`) preserve each argument value
(`f(1,2,3)`). Arguments are pushed in reverse order to respect the
course requirements.

\newpage
=====================================================================
Other Notes & Micro-Optimizations
=====================================================================

* Parameter evaluation order is enforced as right-to-left.
* Invariant checking added via asserts and fail-fast.
* Parser contexts track calls/braces for better error recovery.
* Diagnostics formatter improved: `^` marks primary, `~` marks spans.
