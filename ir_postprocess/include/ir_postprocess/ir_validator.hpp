#ifndef IR_VALIDATOR_HPP
#define IR_VALIDATOR_HPP
#include <vector>

#include "core/ir/ir_quad.hpp"

namespace alpha
{
namespace settings
{
    struct ConfigFlags;
}

class DiagnosticReporter;

class IRValidator
{
public:
    IRValidator(const settings::ConfigFlags& config_flags, DiagnosticReporter& dr);

    void run(const ir::QuadStream& quads);

private:
    const settings::ConfigFlags& config_flags_;
    DiagnosticReporter& dr_;


    void check_uninitialized_reads(const ir::QuadStream& quads);
};
} // namespace alpha

#endif // IR_VALIDATOR_HPP
