#ifndef CLI_SERVER_HPP
#define CLI_SERVER_HPP

namespace vc {
namespace cli {

class Server
{
public:
    virtual int output(const char *buf, uint16_t length) = 0;

    virtual int output_format(const char *fmt, ...) = 0;
};

} // namespace cli
} // namespace vc

#endif /* CLI_SERVER_HPP */
