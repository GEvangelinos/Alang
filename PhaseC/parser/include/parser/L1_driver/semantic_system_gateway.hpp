#ifndef SEMANTIC_SYSTEM_GATEWAY_HPP
#define SEMANTIC_SYSTEM_GATEWAY_HPP

namespace alpha
{
enum class SemanticSystemStatus :std::uint8_t { OK, ERROR };

class SemanticSystem;

class SemanticSystemGateway
{
public:
    void set_error_state() noexcept { ss_status_ = SemanticSystemStatus::ERROR; }

private:
    SemanticSystemStatus &ss_status_;

    explicit SemanticSystemGateway(SemanticSystemStatus &ss_status) : ss_status_(ss_status) {}

    friend class SemanticSystem;
};
} // namespace alpha
#endif // SEMANTIC_SYSTEM_GATEWAY_HPP
