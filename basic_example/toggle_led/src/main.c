#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/shell/shell.h>

#define SLEEP_TIME_MS   1000

#define LED0_NODE DT_ALIAS(builtin_led)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

// Create the Shell Command

// SHELL_CMD_ARG_REGISTER(syntax, subcmd, help, handler, 0, 0)

// Parameters
// [in]	syntax	Command syntax (for example: history).
// [in]	subcmd	Pointer to a subcommands array.
// [in]	help	Pointer to a command help string. Use SHELL_HELP for structured help.
// [in]	handler	Pointer to a function handler.

// Define Handler
static int cmd_led_toggle(const struct shell *sh, size_t argc, char **argv) {
    gpio_pin_toggle_dt(&led);
    shell_print(sh, "LED Toggled!");
    return 0; // Return 0 on success
}

SHELL_CMD_REGISTER(blink_led, NULL, "Help -> Toggle Built In LED", cmd_led_toggle);

int main()
{
    if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

    int ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

    k_msleep(SLEEP_TIME_MS);
    return 0;
}

