#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

static int cmd_tb_ping(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    shell_print(sh, "PONG");
    return 0;
}

SHELL_CMD_REGISTER(tb_ping, NULL, "Test Bench ping command", cmd_tb_ping);

int main(void)
{
    printk("Test Bench Firmware Started\n");
    return 0;
}