#ifndef IR_VALIDATOR_HPP
#define IR_VALIDATOR_HPP
#include <vector>

namespace alpha
{
namespace settings
{
    struct ConfigFlags;
}

namespace ir
{
    struct Quad;
}

class DiagnosticReporter;

class IRValidator
{
public:
    IRValidator(const settings::ConfigFlags& config_flags, DiagnosticReporter& dr);

    void run(const std::vector<ir::Quad>& quads);

private:
    const settings::ConfigFlags& config_flags_;
    DiagnosticReporter& dr_;


    void check_uninitialized_reads(const std::vector<ir::Quad>& quads);
};
} // namespace alpha

#endif // IR_VALIDATOR_HPP
