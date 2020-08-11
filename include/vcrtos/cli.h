#ifndef VCRTOS_CLI_H
#define VCRTOS_CLI_H

#include <stdarg.h>
#include <stdint.h>

#include <vcrtos/config.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cli_command
{
    const char *name;
    void (*command_handler_func)(int argc, char *argv[]);
} cli_command_t;

void vccli_uart_init(void *instance);

void vccli_set_user_commands(const cli_command_t *user_commands, uint8_t length);

void vccli_output_bytes(const uint8_t *bytes, uint8_t length);

void vccli_output_format(const char *fmt, ...);

void vccli_output(const char *string, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* VCRTOS_CLI_H */
