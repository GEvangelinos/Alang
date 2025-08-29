#ifndef DRIVER_KONSTANTS_HPP
#define DRIVER_KONSTANTS_HPP

namespace alpha
{
static constexpr char k_symbol_table_exports_dirname[] = "SYMBOL_TABLE_EXPORTS";
static constexpr char k_compile_error_exports_dirname[] = "COMPILE_ERROR_EXPORTS";
static constexpr char k_ir_exports_dirname[] = "IR_EXPORTS";
inline constexpr char k_symbol_table_export_ext[] = ".st.csv";
inline constexpr char k_ir_export_ext[] = ".ir.csv";
inline constexpr char k_diagnostic_export_ext[] = ".diag.csv";
static constexpr char k_symbol_table_csv_export_header[] = "symbol,type,line,scope\n";
static constexpr char k_ir_csv_export_header[] =
    "quad_no,opcode,result,arg1,arg2,label,first_lineno,last_lineno\n";
static constexpr char k_error_csv_export_header[] = "line,column,diagnostic_type,message\n";
} // namespace alpha

#endif // DRIVER_KONSTANTS_HPP
