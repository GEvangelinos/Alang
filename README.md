# Alang – A Complete Compiler Toolchain for the Alpha Programming Language

Alang is a full-featured compiler and runtime system for the **Alpha** programming language, built from scratch in C++. It implements a traditional compiler pipeline with integrated semantic analysis, intermediate representation (IR) generation, bytecode emission, and a custom virtual machine for execution.

**Current Status:** Version 1.2 • Actively developed

---

## Table of Contents

- [Quick Start](#quick-start)
- [What's Inside](#whats-inside)
- [Architecture](#architecture)
- [Building](#building)
- [Usage](#usage)
- [Directory Structure](#directory-structure)
- [Development](#development)
- [Contributing](#contributing)

---

## Quick Start

### Prerequisites

- **C++20** compiler (Clang or GCC)
- **CMake** 3.18+
- **Python 3.6+**
- **Bison** (parser generator)
- **Flex** (optional, for scanner generation)
- **Valgrind** (optional, for memory profiling)

### Build

```bash
git clone https://github.com/GEvangelinos/Alang.git
cd Alang
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Compile & Run

```bash
# Compile Alpha source to bytecode
./executables/alang --source ../program.asc --show_ir --show_abc

# Execute bytecode
./executables/avm --source program.abc
```

### Run Tests

```bash
make regression_check              # Standard regression tests
make regression_check_full         # Full tests with memory checking
```

---

## What's Inside

**Alang** is a complete compiler infrastructure with:

- 🔍 **Full lexical analysis** – Hand-written or Flex-generated scanner with precise token tracking
- 📝 **Syntactic & semantic analysis** – Single-pass LALR(1) parser built with Bison, integrated semantic checking
- 🎯 **Intermediate representation** – Custom IR with optimization passes
- 📦 **Bytecode generation** – Serializable bytecode format (ABC) for portable execution
- ⚙️ **Virtual machine** – Stack-based VM with ALU, memory management, and runtime support
- 🔧 **Comprehensive diagnostics** – Fine-grained error classification (warnings, soft/hard errors, fatals) with excellent error messages
- 🧪 **Regression testing** – Golden test suite with symbol table and IR validation

---

## Architecture

### Compiler Pipeline

```
┌─────────────────────────────────────────────────────────────────┐
│                         Driver (Main)                            │
│                  Orchestrates compilation                        │
└─────────────────────────────────────────────────────────────────┘
                              ↓
                    ┌──────────────────┐
                    │    Scanner       │
                    │ (Lexical)        │
                    └──────────────────┘
                              ↓
                    ┌──────────────────┐
                    │    Parser        │
                    │ (Syntax + Sem.)  │
                    └──────────────────┘
                              ↓
                    ┌──────────────────┐
                    │  IR Gen + Symbol │
                    │    Table         │
                    └──────────────────┘
                              ↓
                    ┌──────────────────┐
                    │  IR Post-Proc.   │
                    │  (Optimization)  │
                    └──────────────────┘
                              ↓
                    ┌──────────────────┐
                    │  Bytecode Gen.   │
                    │  (ABC Format)    │
                    └──────────────────┘
                              ↓
                    ┌──────────────────┐
                    │  VM Execution    │
                    │  (Runtime)       │
                    └──────────────────┘
```

### Key Design Principles

**Centralized Error Handling:** The Driver acts as the "conductor" — subsystems emit diagnostics but don't decide policy. This ensures uniform error behavior across the pipeline and makes it trivial to adjust global error policy.

**Single-Pass Compilation:** Syntax and semantic analysis happen together in one pass, reducing memory overhead and enabling incremental error recovery.

**Code Generation at Build Time:** Parser specs, IR opcodes, and diagnostics are specified declaratively (YAML/Bison) and code-generated to C++ at build time, keeping implementations DRY.

---

## Building

### Standard Release Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Build Options

| Option | Default | Purpose |
|--------|---------|---------|
| `-DDEBUG_MODE=ON` | OFF | Enable debug checks, RTTI, debug symbols |
| `-DOPTIMIZED_MODE=ON` | OFF | Enable O3, native arch, disable RTTI |
| `-DUSE_FLEX_SCANNER=ON` | OFF | Use Flex-generated scanner instead of hand-written |
| `-DPARSER_STACK_CAPACITY=<N>` | 1000 | Set parser stack size |
| `-DENABLE_IWYU=ON` | OFF | Enable include-what-you-use integration |
| `-DHATE_PYTHON_MODE=ON` | OFF | Disable Python generators, use cached output |

### Example: Debug Build with Full Checks

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DDEBUG_MODE=ON
cmake --build .
```

**Note:** `-DDEBUG_MODE` and `-DOPTIMIZED_MODE` are mutually exclusive (DEBUG_MODE requires RTTI).

---

## Usage

### Compiler (alang)

```bash
./executables/alang [OPTIONS] --source <file.asc>
```

#### Common Options

- `--source <file>` – Input Alpha source file (required)
- `--show_tokens` – Perform lexical analysis only
- `--show_ir` – Print generated intermediate representation
- `--show_abc` – Print generated bytecode
- `--show_symbol_table` – Print final symbol table
- `--export_symbol_table` – Export symbol table to file
- `--export_diagnostics` – Export diagnostics to file

Run `./executables/alang --help` for complete option list.

#### Example

```bash
# Compile and see all stages
./executables/alang --source program.asc --show_tokens --show_ir --show_abc

# Compile and export diagnostics
./executables/alang --source program.asc --export_diagnostics diags.txt
```

### Virtual Machine (avm)

```bash
./executables/avm --source <file.abc>
```

Executes a compiled `.abc` bytecode file.

---

## Directory Structure

```
├── CMakeLists.txt              Root build configuration
├── README.md                   This file
├── REPO_OVERVIEW.md            Detailed architecture documentation
│
├── core/                       IR data structures, AST, symbol tables
│   ├── include/core/           Public headers (IR nodes, types, etc.)
│   └── src/                    Implementation
│
├── scanner/                    Lexical analysis
│   ├── include/scanner/        Scanner interfaces
│   └── src/                    Scanner automaton, handwritten or Flex-based
│
├── parser/                     Syntax & semantic analysis
│   ├── src/
│   │   ├── alpha_parser_spec.y       Bison grammar specification
│   │   ├── symbol_table.cpp          Symbol table implementation
│   │   ├── L1_driver/                High-level semantic coordination
│   │   └── L2_builders/              Semantic action implementations
│   │       ├── expr_builders.cpp     Expression semantic rules
│   │       ├── lvalue_resolver.cpp   L-value detection & resolution
│   │       ├── control_flow_manager.cpp  Loop/conditional handling
│   │       ├── expr_normalizer.cpp   Expression tree normalization
│   │       ├── expr_optimizer.cpp    Expression-level optimizations
│   │       └── core/                 Intermediate code generation
│   ├── config/                Code generation specifications
│   │   └── ir_opcode_spec.yaml       IR opcode definitions
│   └── scripts/               Build-time code generators
│       ├── log_injector/      Inject trace calls into parser
│       └── ir_opcode_generator/  Generate IR opcodes from YAML
│
├── bytecode/                   Bytecode (ABC) format & I/O
│   ├── include/bytecode/       Bytecode format, serializer, loader
│   └── src/
│       ├── abc_generator.cpp   Convert IR → ABC bytecode
│       ├── abc_serializer.cpp  Write bytecode to disk
│       └── abc_loader.cpp      Load bytecode from disk
│
├── vm/                         Virtual machine execution
│   ├── include/vm/             VM interfaces
│   └── src/
│       ├── machine.cpp         Instruction dispatch loop
│       ├── machine_alu.cpp     Arithmetic/logic operations
│       └── vm_memory.cpp       Memory management
│
├── diagnostics/                Error/warning message system
│   ├── config/
│   │   └── diagnostics_spec.yaml   Diagnostic message specifications
│   ├── scripts/                Code generator for diagnostics
│   └── src/                    Diagnostic emission & formatting
│
├── driver/                     Main compiler entry point
│   ├── include/driver/         Driver interfaces
│   └── src/
│       └── main.cpp            Entry point
│
├── support/                    Utilities
│   ├── include/support/        Common utilities, CLI colors
│   └── src/
│
├── arguinator/                 CLI argument parsing
│   └── include/arguinator/     Argument parser interface
│
├── ir_postprocess/             IR optimization passes
│
├── settings/                   Compiler configuration options
│
├── scripts/                    Utility scripts
│   ├── old_regression_runner/  Legacy test infrastructure
│   └── prophet/                Current test harness
│
├── .tests/                     Regression test cases (Alpha source)
├── .tests_phase3/              Phase 3 test suite
├── .tests_phase5/              Phase 5 test suite
├── .GOSPEL/                    Golden expected outputs
│
├── docs/                       Documentation
│   ├── core_ideas_history.txt  Design evolution & philosophy
│   ├── naming_conventions.md   Coding style guide
│   └── naming_conventions.md   Code organization notes
│
└── third_party/                External dependencies
```

---

## Development

### Code Organization Philosophy

- **Single Responsibility:** Each module has one clear purpose
- **Header/Implementation Split:** Public APIs in `include/`, implementations in `src/`
- **Semantic Builders:** Complex semantic actions grouped by concern (expressions, control flow, etc.)
- **YAML → C++:** Specs (opcodes, diagnostics) live in YAML; code generators produce headers/implementations

### Build-Time Code Generation

Several components are generated at build time:

1. **Parser with Trace Injection:** `scripts/log_injector/main.py` injects trace calls into Bison spec
2. **IR Opcodes:** `scripts/ir_opcode_generator/main.py` generates opcode enums and info tables from `ir_opcode_spec.yaml`
3. **Diagnostics:** `diagnostics/scripts/` generates diagnostic enums and formatters from `diagnostics_spec.yaml`

These keep specifications maintainable while avoiding boilerplate.

### Diagnostic System

The diagnostic system classifies errors into four levels:

- **Warning:** Non-blocking informational message
- **Soft Error:** Reportable error; doesn't halt semantic analysis
- **Hard Error:** Breaks semantic flow; halts analysis until recovery point
- **Fatal Error:** Unrecoverable; escapes compilation phase

Subsystems (scanner, parser, semantic analyzer) emit and classify; the **Driver** decides policy. This design allows multiple errors to be reported in a single compilation pass.

### Adding a New Diagnostic

1. Add entry to `diagnostics/config/diagnostics_spec.yaml`
2. Rebuild: `cmake --build .` (triggers code generation)
3. Use in code: `emit_soft_error(DiagnosticId::YourNewDiag, location, args)`

### Naming Conventions

See `docs/naming_conventions.md`:

- Private class members end with `_`
- Type names use `CamelCase`
- Function names use `snake_case`
- Constants use `SCREAMING_SNAKE_CASE`

### Testing

#### Run All Tests

```bash
cd build
make regression_check
```

#### Run with Memory Checking (Valgrind)

```bash
make regression_check_full
```

#### Add a New Test

1. Create a `.asc` file in `.tests/`
2. Run the compiler and commit golden outputs to `.GOSPEL/`
3. Tests are automatically picked up by the regression runner

#### Test Structure

- `.tests/*.asc` – Test source files
- `.GOSPEL/` – Golden expected outputs (symbol tables, IR, diagnostics)
- `scripts/prophet/prophet.py` – Test harness; compares outputs

---

## Project Status

- ✅ Lexical analysis (scanner with error recovery)
- ✅ Syntactic analysis (Bison LALR(1) parser)
- ✅ Semantic analysis (single-pass with integrated checking)
- ✅ Intermediate representation (IR nodes, optimization framework)
- ✅ Bytecode generation & serialization
- ✅ Virtual machine with ALU & memory model
- ✅ Comprehensive error diagnostics
- ✅ Regression testing infrastructure
- 🔄 Continuous refinement & optimization

---

## Contributing

Contributions are welcome! Before starting, please:

1. Read `docs/naming_conventions.md` for style guidelines
2. Check `docs/core_ideas_history.txt` for design philosophy
3. Run `make regression_check` to ensure existing tests pass
4. Add tests for new features in `.tests/`

---

## License

This project is unlicensed. See repository settings for details.

---

## Questions?

- 📖 Start with `REPO_OVERVIEW.md` for architecture deep-dive
- 💬 Check `docs/core_ideas_history.txt` for design rationale
- 🔍 Explore test cases in `.tests/` to understand the language
- 💻 Examine `driver/src/main.cpp` to trace the compilation flow

---

**Built with ❤️ for compiler enthusiasts and language designers.**
