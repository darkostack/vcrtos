#ifndef CLI_UART_HPP
#define CLI_UART_HPP

#include <vcrtos/config.h>

#include "cli/cli.hpp"
#include "cli/cli_server.hpp"

namespace vc {

class Instance;

namespace cli {

class Uart : public Server
{
public:
    explicit Uart(Instance &instances);

    virtual int output(const char *buf, uint16_t buf_length);

    virtual int output_format(const char *fmt, ...);

    Interpreter &get_interpreter(void) { return _interpreter; }

    void receive_task(const uint8_t *buf, uint16_t buf_length);

    static Uart *_uart_server;

private:
    enum
    {
        RX_BUFFER_SIZE = VCRTOS_CONFIG_CLI_UART_RX_BUFFER_SIZE,
        TX_BUFFER_SIZE = VCRTOS_CONFIG_CLI_UART_TX_BUFFER_SIZE,
        MAX_LINE_LENGTH = VCRTOS_CONFIG_CLI_MAX_LINE_LENGTH,
    };

    int process_command(void);
    void send(void);

    char _rx_buffer[RX_BUFFER_SIZE];
    uint16_t _rx_length;

    char _tx_buffer[TX_BUFFER_SIZE];
    uint16_t _tx_head;
    uint16_t _tx_length;

    uint16_t _send_length;

    Interpreter _interpreter;

    friend class Interpreter;
};

} // namespace cli
} // namespace vc

#endif /* CLI_UART_HPP */
