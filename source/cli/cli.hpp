#ifndef CLI_HPP
#define CLI_HPP

#include <stdint.h>

#include <vcrtos/config.h>
#include <vcrtos/cli.h>

namespace vc {

class Instance;

#if !VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
extern uint64_t instance_raw[];
#endif

namespace cli {

class Interpreter;
class Server;

class Interpreter
{
public:
    explicit Interpreter(Instance &instances);

    void set_user_commands(const cli_command_t *commands, uint8_t length);

    void process_line(char *buf, uint16_t length, Server &server);

    static int parse_long(char *string, long &result);

    static int parse_unsigned_long(char *string, unsigned long &result);

    void output_bytes(const uint8_t *bytes, uint8_t length) const;

private:
    enum
    {
        MAX_ARGS = 32,
    };

    template <typename Type> inline Type &get(void) const;

#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    Instance &get_instance(void) const { return *static_cast<Instance *>(instance); }
#else
    Instance &get_instance(void) const { return *reinterpret_cast<Instance *>(&instance_raw); }
#endif

    const cli_command_t *_user_commands;

    uint8_t _user_commands_length;

    Server *_server;

    void *instance;
};

} // namespace cli
} // namespace vc

#endif /* CLI_HPP */
