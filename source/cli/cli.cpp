#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/code_utils.h"
#include "core/instance.hpp"

#include "cli/cli.hpp"
#include "cli/cli_server.hpp"

namespace vc {
namespace cli {

Interpreter::Interpreter(Instance &instances)
    : _user_commands(NULL)
    , _user_commands_length(0)
    , _server(NULL)
{
#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    instance = static_cast<void *>(&instances);
#else
    (void) instances;
#endif
}

void Interpreter::output_bytes(const uint8_t *bytes, uint8_t length) const
{
    for (int i = 0; i < length; i++)
    {
        _server->output_format("%02x", bytes[i]);
    }
}

int Interpreter::parse_long(char *string, long &result)
{
    char *endptr;
    result = strtol(string, &endptr, 0);
    return (*endptr == '\0') ? 1 : 0;
}

int Interpreter::parse_unsigned_long(char *string, unsigned long &result)
{
    char *endptr;
    result = strtoul(string, &endptr, 0);
    return (*endptr == '\0') ? 1 : 0;
}

void Interpreter::process_line(char *buf, uint16_t length, Server &server)
{
    char *argv[MAX_ARGS];
    char *cmd;
    uint8_t argc = 0, i = 0;

    _server = &server;

    VERIFY_OR_EXIT(buf != NULL);

    for (; *buf == ' '; buf++, length--);

    for (cmd = buf + 1; (cmd < buf + length) && (cmd != NULL); ++cmd)
    {
        VERIFY_OR_EXIT(argc < MAX_ARGS);

        if (*cmd == ' ' || *cmd == '\r' || *cmd == '\n')
        {
            *cmd = '\0';
        }

        if (*(cmd - 1) == '\0' && *cmd != ' ')
        {
            argv[argc++] = cmd;
        }
    }

    cmd = buf;

    VERIFY_OR_EXIT(_user_commands != NULL && _user_commands_length != 0);

    for (i = 0; i < _user_commands_length; i++)
    {
        if (strcmp(cmd, _user_commands[i].name) == 0)
        {
            _user_commands[i].command_handler_func(argc, argv);
            break;
        }
    }

    if (i == _user_commands_length)
    {
        _server->output_format("Unknown command: %s\r\n", cmd);
    }
    else
    {
        _server->output_format("Done\r\n");
    }

exit:
    return;
}

void Interpreter::set_user_commands(const cli_command_t *commands, uint8_t length)
{
    _user_commands = commands;
    _user_commands_length = length;
}

template <> inline Instance &Interpreter::get(void) const
{
    return get_instance();
}

template <typename Type> inline Type &Interpreter::get(void) const
{
    return get_instance().get<Type>();
}

} // namespace cli
} // namespace vc
