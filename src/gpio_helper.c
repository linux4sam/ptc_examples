#include <stdio.h>
#include <stdlib.h>
#include <gpiod.h>

#include "gpio_helper.h"

static struct gpiod_chip *gpiochip = NULL;

int gpio_init()
{
	if (!gpiochip) {
		gpiochip = gpiod_chip_open("/dev/gpiochip0");
		if (!gpiochip) {
			fprintf(stderr, "gpiod_chip_open failed\n");
			return -1;
		}
	}

	return 0;
}

void gpio_fini()
{
	if (gpiochip)
		gpiod_chip_close(gpiochip);
}

int gpio_led_request(struct gpio_led_desc *led)
{
	struct gpiod_request_config *req_cfg;
	struct gpiod_line_request *request = NULL;
	struct gpiod_line_settings *settings;
	struct gpiod_line_config *line_cfg;
	int ret = -1;

	if (!led)
		return ret;

	settings = gpiod_line_settings_new();
	if (!settings)
		return ret;

	if (gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT))
		goto free_settings;

	gpiod_line_settings_set_active_low(settings, true);

	if (gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE))
		goto free_settings;

	line_cfg = gpiod_line_config_new();
	if (!line_cfg)
		goto free_settings;

	if (gpiod_line_config_add_line_settings(line_cfg, &led->pin_id, 1, settings))
		goto free_line_config;

	req_cfg = gpiod_request_config_new();
	if (!req_cfg)
		goto free_line_config;

	gpiod_request_config_set_consumer(req_cfg, "ptc qt example");

	led->gpio_line = gpiod_chip_request_lines(gpiochip, req_cfg, line_cfg);
	if (led->gpio_line)
		ret = 0;

	gpiod_request_config_free(req_cfg);

free_line_config:
	gpiod_line_config_free(line_cfg);

free_settings:
	gpiod_line_settings_free(settings);

	return ret;
}

void gpio_led_release(struct gpio_led_desc *led)
{
	if (led && led->gpio_line)
		gpiod_line_request_release(led->gpio_line);
}

int gpio_led_on(struct gpio_led_desc *led)
{
	if (!led || !led->gpio_line)
		return -1;

	return gpiod_line_request_set_value(led->gpio_line, led->pin_id, GPIOD_LINE_VALUE_ACTIVE);
}

int gpio_led_off(struct gpio_led_desc *led)
{
	if (!led->gpio_line)
		return -1;

	return gpiod_line_request_set_value(led->gpio_line, led->pin_id, GPIOD_LINE_VALUE_INACTIVE);
}

