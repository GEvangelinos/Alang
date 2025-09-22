
# Naming Conventions — Alpha Compiler (Semantic & IR Builders)

**Purpose:** Make names self-explanatory, consistent, and easy to grep.  
**Scope:** Parser semantic actions, builder subsystems, control-flow managers, and IR emission within `alpha`.

---

## 1) Function name prefixes

| Prefix        | Meaning / Contract | Returns | IR Emission | Typical Caller |
|---|---|---:|:--:|---|
| `build_`      | Construct an IR node/Expr or a multi-instruction fragment representing a source construct. Use when there is a semantic “thing” being produced. | `Expr*` or related handle (preferred), but may be `void` for pure control-flow phases | ✅ allowed | Grammar via dispatcher |
| `emit_`       | Perform side-effectful IR emission (no value produced). | `void` | ✅ | Grammar or other builders |
| `process_`    | Handle a high-level semantic event with validation + IR (e.g., loop keywords). | `void`/`bool` | ✅ | Grammar via dispatcher |
| `validate_`   | Check preconditions, produce diagnostics, **no IR**. | `bool` | ❌ | Builders / helpers |
| `handle_`     | Internal sub-step of a `build_`/`process_` pipeline (not dispatcher-visible). | varies | ✅ allowed | Private helpers only |
| `try_optimize_` | Attempt constant folding/peepholes; may return optimized `Expr*` or `nullptr`. | `Expr*`/`nullptr` | usually ❌ | Builders / optimizer |
| `update_`     | Mutate transient draft state (e.g., method name buffer). | `void` | ❌ | Grammar via dispatcher |
| `collect_`    | Append an item into a draft list (e.g., parameters). | `void` | ❌ | Grammar via dispatcher |
| `register_`   | Commit draft items into tables/scopes. | `void` | ❌ | Builders only |
| `mark_`       | Record a label/position for later patching. | `void` | maybe | Builders / managers |
| `patch_`      | Resolve recorded labels to the current position. | `void` | ✅ | Builders / managers |

**Project examples:**  
- `basic_MODEer.build_arithmetic`, `basic_MODEer.build_relational`, `basic_MODEer.build_uminus`  
- `assign_MODEer.build_assignment`, `build_pre_inc`, `build_post_dec`  
- `table_access_MODEer.build_member_access`, `build_index_access`  
- `aggregate_MODEer.build_table_list_consuming`, `build_expr_pair`  
- `call_MODEer.update_method_call_draft`, `build_method_call_consuming`  
- `function_MODEer.update_function_draft`, `collect_function_parameter`, `build_program_function_entry/exit`  
- `loop_manager.process_break`, `process_continue`  
- `basic_MODEer.mark_short_circuit_jump_point`

---

## 2) Suffixes

| Suffix | Meaning / Ownership rules |
|---|---|
| `_consuming` | Callee **takes ownership** of pointer/list arguments and **nullifies** them after use. Caller must not reuse. Example: `build_table_list_consuming(ExprList*& elist, loc)` deletes/sets `elist=nullptr`. |
| `_entry` | Start phase of a multi-phase construct; sets up state/labels. Pairs with `_exit`. |
| `_exit` | End phase; patches pending labels, finalizes state. |
| `_list` / `_pair` | Operate on collection elements for literals (tables/dicts). |
| `_draft` | Transient, grammar-fed state that will later be committed. Used with `update_` / `collect_` / `register_`. |

---

## 3) Ownership & lifetime rules

- Functions with `_consuming` **must** document which parameters are consumed and guarantee nullification (`ptr = nullptr`) after deletion.  
- `build_...` **returns** a heap-managed IR node/Expr unless clearly a control-flow phase (then prefer `emit_...`).  
- No function may silently delete objects it didn’t allocate **unless** it is `_consuming` by contract.  
- Shallow vs deep deletes must be explicit in comments (e.g., dict list deletes pairs but **not** the inner Exprs).

---

## 4) Grammar integration & dispatcher visibility

- Grammar actions **only** call dispatcher-visible functions declared with `DISPATCH_SLAVE_METHOD_CALL`.  
- Internal helpers stay `private` and use `handle_`/`validate_`/`try_optimize_` prefixes.  
- Use declarative, readable calls from grammar, e.g.:  
  `ss.call<"basic_MODEer.build_arithmetic">(Op::ADD, $lhs, $rhs, @out);`

**Boolean contexts:** Grammar may remain permissive (`expr` on LHS of `=`); semantic builders perform lvalue checks and emit precise diagnostics.

---

## 5) Control-flow conventions

**If/else**  
- Entry (condition lowering + fall-through/skip labels): `emit_if_branch_entry` **or** `build_if_branch_entry` (project-chosen).  
- Else/End patches: `patch_if_else_entry`, `patch_if_end` (example names; keep `patch_` prefix for resolution).

**Loops**  
- Enter/exit depth: `enter_loop`, `exit_loop`.  
- Continue target: `patch_continue_here` (placed at condition for `while`, at increment for `for`).  
- Break target: `patch_break_here` (placed after loop).  
- Loop keywords: `process_break`, `process_continue` (validate + emit labelless `JUMP`).

---

## 6) Error & validation

- `validate_...` functions return `bool` and **never** emit IR. They report via `dr_`.  
- Builders call `validate_` early, and short-circuit on failure (return `nullptr` or a poison `ERROR_EXPR` depending on policy).  
- Diagnostics should include the operator’s display string and operand kinds.  
  Examples:  
  - `validate_lvalue_assignment` → special-cases library/program functions.  
  - `validate_arithmetic_expr` / `validate_relational_expr` → report side (LEFT/RIGHT) and opcode string.

---

## 7) Short-circuit boolean lowering

- Mark points with `mark_short_circuit_jump_point`.  
- Use policy structs to pick true/false lists to patch/merge.  
- Functions that assume a `BoolExpr` must **either** call optimizer first **or** type-check before static_cast.

---

## 8) Collections & table literals

- **List literals:** forward iteration preferred; index base is explicit (0-based unless specified).  
- **Dict literals:** iteration order is explicit; if “last write wins” is a rule, keep emission order consistent.  
- Names clearly reflect ownership: `build_expr_list`, `extend_expr_list`, `delete_expr_list`, `build_dict_list`, `extend_dict_list`, `delete_dict_list`.

---

## 9) Recommended pairs & examples

- `build_if_branch_entry` ↔ `patch_if_end` (or `emit_if_conditional_branch` ↔ `patch_if_end`)  
- `build_program_function_entry` ↔ `build_program_function_exit`  
- `enter_loop` ↔ `exit_loop`  
- `mark_short_circuit_jump_point` ↔ (implicit patching in `build_logical_and/or`)  
- `build_method_call_consuming` (implicitly consumes arg list).

---

## 10) Consistency checklist (PR reviewer quick pass)

- [ ] Prefix matches behavior (`build_` returns value unless control-flow phase).  
- [ ] `_consuming` present if ownership is transferred, and arguments are nullified.  
- [ ] `validate_` contains **no** IR emission.  
- [ ] Dispatcher exposes only functions intended for grammar use.  
- [ ] Control-flow functions use `emit_`/`patch_`/`process_` prefixes appropriately.  
- [ ] Comments state shallow vs deep delete behavior for containers.  
- [ ] Boolean short-circuit builders guard casts or rely on guaranteed folding.

---

## 11) Glossary

- **IR emission**: Appending quads via `quad_handler_`.  
- **Label patching**: Recording `next_quad_label()` before `JUMP` and later resolving to a concrete label with `patch_*`.  
- **Draft state**: Transient fields (e.g., method/function name + param list) filled by grammar before commit.

---

### Notes aligned to current codebase

- Prefer `emit_` for purely side-effect control-flow steps (e.g., the function currently named `ifPrefix__if_lparen_expr_rparen` → `emit_if_conditional_branch` or `build_if_branch_entry` per project policy).  
- Keep `_consuming` semantics for lists/dicts as already implemented in `AggregateBuilder`.
- Keep `%right ASSIGN` in grammar; rely on `validate_lvalue_assignment` for diagnostics.

