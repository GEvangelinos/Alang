#ifndef SEMANTIC_SYSTEM_GATEWAY_HPP
#define SEMANTIC_SYSTEM_GATEWAY_HPP

class SemanticSystemGateway
{
public:
    explicit SemanticSystemGateway(bool &in_error) : in_error_(in_error) {}

    void set_error_state() noexcept { in_error_ = true; }

private:
    bool &in_error_;
    friend class SemanticSystem;
    friend class DiagnosticEngine;
};
#endif // SEMANTIC_SYSTEM_GATEWAY_HPP
