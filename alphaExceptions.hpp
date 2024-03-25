#include <stdexcept>

namespace Alpha
{
        class UnexpectedEOF : public std::runtime_error
        {
        public:
                UnexpectedEOF(std::string runtimeMessage)
                    : std::runtime_error(runtimeMessage)
                {
                }
        };

} /* namespace Alpha */