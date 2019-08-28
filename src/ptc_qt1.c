/*
 * Demo for PTC QT1 mutual and self capacitance wings.
 *
 * Copyright 2018 Microchip
 * 		  Ludovic Desroches <ludovic.desroches@microchip.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libevdev-1.0/libevdev/libevdev.h>
#include <gpiod.h>

#include "ptc_qt.h"

#define BUTTONS_INPUT_FILE	"/dev/input/atmel_ptc0"
#define SLIDER_INPUT_FILE	"/dev/input/atmel_ptc1"
#define WHEEL_INPUT_FILE	"/dev/input/atmel_ptc2"
#define POLL_NFDS		3

#ifdef MUTCAP
#define NUMBER_OF_BUTTONS	2
#define SLIDER_NB_OF_LEDS	7
#define WHEEL_NB_OF_LEDS	3

static unsigned int buttons_keycodes[NUMBER_OF_BUTTONS] = {
	0x108, 0x109,
};

static struct led_desc buttons_leds[NUMBER_OF_BUTTONS] = {
	{ .pin_id = 103 },	/* PD7 */
	{ .pin_id = 104 },	/* PD8 */
};

static struct led_desc slider_leds[SLIDER_NB_OF_LEDS] = {
	{ .led_id = 0, .pin_id = 107 },	/* PD11 */
	{ .led_id = 1, .pin_id = 108 },	/* PD12 */
	{ .led_id = 2, .pin_id = 41 },	/* PB9 */
	{ .led_id = 3, .pin_id = 64 },	/* PC0 */
	{ .led_id = 4, .pin_id = 113 },	/* PD17 */
	{ .led_id = 5, .pin_id = 114 },	/* PD18 */
	{ .led_id = 6, .pin_id = 122 },	/* PD26 */
	/* unused */			/* PD17 */
};

static struct led_desc wheel_leds[WHEEL_NB_OF_LEDS] = {
	{ .pin_id = 105 },	/* PD9 */
	{ .pin_id = 106 },	/* PD10 */
	{ .pin_id = 57 },	/* PB25 */
};
#endif /* MUTCAP */

#ifdef SELFCAP
#define NUMBER_OF_BUTTONS	1
#define SLIDER_NB_OF_LEDS	8
#define WHEEL_NB_OF_LEDS	3

static unsigned int buttons_keycodes[NUMBER_OF_BUTTONS] = {
	0x106,
};

static struct led_desc buttons_leds[1] = {
	{ .pin_id = 104},	/* PD8 */
};

static struct led_desc slider_leds[SLIDER_NB_OF_LEDS] = {
	{ .led_id = 0, .pin_id = 41 },	/* PB9 */
	{ .led_id = 1, .pin_id = 64 },	/* PC0 */
	{ .led_id = 2, .pin_id = 113 },	/* PD17 */
	{ .led_id = 3, .pin_id = 122 },	/* PD26 */
	{ .led_id = 4, .pin_id = 99 },	/* PD3 */
	{ .led_id = 5, .pin_id = 100 },	/* PD4 */
	{ .led_id = 6, .pin_id = 101 },	/* PD5 */
	{ .led_id = 7, .pin_id = 102 },	/* PD6 */
};

static struct led_desc wheel_leds[WHEEL_NB_OF_LEDS] = {
	{ .pin_id = 105 },	/* PD9 */
	{ .pin_id = 106 },	/* PD10 */
	{ .pin_id = 57 },	/* PB25 */
};
#endif /* SELFCAP */

struct gpiod_chip *gpiochip;

static int button_event_handler(struct buttons *buttons)
{
	struct input_event ev;
	int i, ret;

	do {
		ret = libevdev_next_event(buttons->evdev,
					  LIBEVDEV_READ_FLAG_NORMAL, &ev);
		if (ret == LIBEVDEV_READ_STATUS_SYNC) {
			fprintf(stderr, "error: cannot keep up\n");
			return -1;
		} else if (ret != -EAGAIN && ret < 0) {
			fprintf(stderr, "error: %s\n", strerror(-ret));
			return -1;
		} else	if (ret == LIBEVDEV_READ_STATUS_SUCCESS) {
			for (i = 0; i < NUMBER_OF_BUTTONS; i++) {
				unsigned int key_code = buttons_keycodes[i];
				struct gpiod_line *led_gpio_line =
					buttons_leds[i].gpio_line;

				if (key_code == ev.code)
					gpiod_line_set_value(led_gpio_line,
							     ev.value);
			}
		}
	} while (ret != -EAGAIN);

	return ret;
}

static void remove_buttons(struct buttons *buttons)
{
	int i;

	for (i = 0; i < NUMBER_OF_BUTTONS; i++) {
		if (buttons_leds[i].gpio_line)
			gpiod_line_release(buttons_leds[i].gpio_line);
	}

	if (buttons->evdev)
		libevdev_free(buttons->evdev);

	if (buttons->fd > 0)
		close(buttons->fd);

	free(buttons);
}

static struct buttons *initialize_buttons(void)
{
	struct buttons *buttons;
	unsigned int i;

	buttons = malloc(sizeof(*buttons));
	if (!buttons) {
		fprintf(stderr, "Can't allocate buttons\n");
		return NULL;
	}

	buttons->fd = open(BUTTONS_INPUT_FILE, O_RDONLY | O_NONBLOCK);
	if (buttons->fd < 0) {
		fprintf(stderr, "Can't open %s\n", BUTTONS_INPUT_FILE);
		goto out;
	}

	if (libevdev_new_from_fd(buttons->fd, &buttons->evdev) < 0) {
		fprintf(stderr, "Can't init libevdev for %s\n", BUTTONS_INPUT_FILE);
		goto out;
	}

	if (strncmp("atmel_ptc", libevdev_get_name(buttons->evdev), strlen("atmel_ptc"))) {
		fprintf(stderr, "%s is not a buttons input device from the PTC\n", BUTTONS_INPUT_FILE);
		goto out;
	}

	for (i = 0; i < NUMBER_OF_BUTTONS; i++) {
		struct gpiod_line *led_gpio_line;
		unsigned int pin_id = buttons_leds[i].pin_id;

		led_gpio_line = gpiod_chip_get_line(gpiochip, pin_id);
		if (!led_gpio_line) {
			fprintf(stderr, "can't get gpio line for button %d led\n", i);
			goto out;
		}
		gpiod_line_request_output_flags(led_gpio_line,
			"ptc qt example", GPIOD_LINE_REQUEST_FLAG_ACTIVE_LOW, 0);
		buttons_leds[i].gpio_line = led_gpio_line;
	}

	return buttons;

out:
	remove_buttons(buttons);
	return NULL;
}

static void slider_position_update(struct led_desc *leds, unsigned int nleds,
				   unsigned int ev_type, unsigned int ev_value,
				   void *arg)
{
	unsigned int i, display_value;
	struct gpiod_line *led_gpio_line;

	if (ev_type == EV_KEY) {
		if (ev_value == 0) {
			for (i = 0; i < nleds; i++) {
				led_gpio_line = leds[i].gpio_line;

				if (led_gpio_line)
					gpiod_line_set_value(led_gpio_line, 0);
			}
		}
	} else if (ev_type == EV_ABS) {
		/*
		 * ev_value range is from 0 to 63 (depends on scroller resolution),
		 * split it into 8 parts for display.
		 */
		display_value = ev_value / 8;
		for (i = 0; i < nleds; i++) {
			struct gpiod_line *led_gpio_line = leds[i].gpio_line;

			if (!led_gpio_line)
				continue;
			if (leds[i].led_id <= display_value)
				gpiod_line_set_value(led_gpio_line, 1);
			else
				gpiod_line_set_value(led_gpio_line, 0);
		}
	}
}

static void wheel_position_update(struct led_desc *leds, unsigned int nleds,
				  unsigned int ev_type, unsigned int ev_value,
				  void *arg)
{
	unsigned int i;
	struct gpiod_line *led_gpio_line;

	if (ev_type == EV_KEY) {
		if (ev_value == 0) {
			for (i = 0; i < nleds; i++) {
				led_gpio_line = leds[i].gpio_line;

				if (led_gpio_line)
					gpiod_line_set_value(led_gpio_line, 0);
			}
		}
	} else if (ev_type == EV_ABS) {
		/*
		 * Values from 0 to 63, split it into 7 parts,
		 * update it if resolution is different.
		 */
		ev_value = ev_value / 10 + 1;
		for (i = 0; i < nleds; i++) {
			led_gpio_line = leds[i].gpio_line;

			if (led_gpio_line)
				gpiod_line_set_value(led_gpio_line,
					     ((ev_value >> i) & 0x1) ? 1 : 0);
		}
	}
}

int main(void)
{
	int ret, i;
	struct buttons *buttons;
	struct scroller *slider, *wheel;
	struct pollfd fds[POLL_NFDS];

	gpiochip = gpiod_chip_open("/dev/gpiochip0");
	if (!gpiochip)
		fprintf(stderr, "gpiod_chip_open failed\n");

	buttons = initialize_buttons();
	if (!buttons)
		return EXIT_FAILURE;

	slider = initialize_scroller(SLIDER_INPUT_FILE, slider_leds,
				     SLIDER_NB_OF_LEDS, gpiochip,
				     slider_position_update);
	if (!slider)
		goto slider_fail;

	wheel = initialize_scroller(WHEEL_INPUT_FILE, wheel_leds,
				    WHEEL_NB_OF_LEDS, gpiochip,
				    wheel_position_update);
	if (!wheel)
		goto wheel_fail;

	fds[0].fd = buttons->fd;
	fds[0].events = POLLIN;
	fds[1].fd = slider->fd;
	fds[1].events = POLLIN;
	fds[2].fd = wheel->fd;
	fds[2].events = POLLIN;

	printf("demo running...\n");
	while (1) {
		ret = poll(fds, POLL_NFDS, -1);
		if (ret < 0) {
			fprintf(stderr, "poll() failed\n");
			break;
		}

		for (i = 0; i < POLL_NFDS; i++) {
			if (fds[i].revents == 0)
				continue;

			if (fds[i].revents != POLLIN) {
				fprintf(stderr, "error, revents = %d\n", fds[i].revents);
				break;
			}

			if (fds[i].fd == buttons->fd) {
				ret = button_event_handler(buttons);
				if (ret)
					break;
			}

			if (fds[i].fd == slider->fd) {
				ret = scroller_event_handler(slider, NULL);
				if (ret)
					break;
			}

			if (fds[i].fd == wheel->fd) {
				ret = scroller_event_handler(wheel, NULL);
				if (ret)
					break;
			}
		}
	}
	fprintf(stderr, "event error\n");

	remove_scroller(wheel);
wheel_fail:
	remove_scroller(slider);
slider_fail:
	remove_buttons(buttons);
	gpiod_chip_close(gpiochip);

	return EXIT_FAILURE;
}
