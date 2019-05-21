/*
 * Library for PTC QTx demos.
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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libevdev-1.0/libevdev/libevdev.h>
#include <gpiod.h>

#include "ptc_qt.h"

int scroller_event_handler(struct scroller *scroller, void *arg)
{
	struct input_event ev;
	int ret;

	do {
		ret = libevdev_next_event(scroller->evdev,
					  LIBEVDEV_READ_FLAG_NORMAL, &ev);
		if (ret == LIBEVDEV_READ_STATUS_SYNC) {
			fprintf(stderr, "error: cannot keep up\n");
			return -1;
		} else if (ret != -EAGAIN && ret < 0) {
			fprintf(stderr, "error: %s\n", strerror(-ret));
			return -1;
		} else	if (ret == LIBEVDEV_READ_STATUS_SUCCESS) {
			scroller->position_update(scroller->leds,
				scroller->nleds, ev.type, ev.value, arg);
		}
	} while (ret != -EAGAIN);

	return 0;
}

void remove_scroller(struct scroller *scroller)
{
	unsigned int i;

	for (i = 0; i < scroller->nleds; i++) {
		if (scroller->leds[i].gpio_line)
			gpiod_line_release(scroller->leds[i].gpio_line);
	}

	if (scroller->evdev)
		libevdev_free(scroller->evdev);

	if (scroller->fd > 0)
		close(scroller->fd);

	free(scroller);
}

struct scroller *initialize_scroller(const char *input_file,
	struct led_desc *leds, unsigned int nleds,
	struct gpiod_chip *gpiochip,
	void (*position_update)(struct led_desc *leds,
				unsigned int nleds,
				unsigned int ev_type,
				unsigned int ev_value,
				void *arg))
{
	struct scroller *scroller;
	unsigned int i;

	scroller = malloc(sizeof(*scroller));
	if (!scroller) {
		fprintf(stderr, "Can't allocate scroller\n");
		return NULL;
	}

	scroller->leds = leds;
	scroller->nleds = nleds;
	scroller->position_update = position_update;

	scroller->fd = open(input_file, O_RDONLY | O_NONBLOCK);
	if (scroller->fd < 0) {
		fprintf(stderr, "Can't open %s\n", input_file);
		goto out;
	}

	if (libevdev_new_from_fd(scroller->fd, &scroller->evdev) < 0) {
		fprintf(stderr, "Can't init libevdev for %s\n", input_file);
		goto out;
	}

	if (strncmp("atmel_ptc", libevdev_get_name(scroller->evdev), strlen("atmel_ptc"))) {
		fprintf(stderr, "%s is not a scroller input device from the PTC\n", input_file);
		goto out;
	}

	for (i = 0; i < nleds; i++) {
		struct gpiod_line *led_gpio_line;
		unsigned int pin_id = scroller->leds[i].pin_id;

		led_gpio_line = gpiod_chip_get_line(gpiochip, pin_id);
		if (!led_gpio_line) {
			fprintf(stderr,
				"can't get gpio line for button %d led\n", i);
			goto out;
		}
		gpiod_line_request_output_flags(led_gpio_line,
			"atqt example", GPIOD_LINE_REQUEST_FLAG_ACTIVE_LOW, 0);
		scroller->leds[i].gpio_line = led_gpio_line;
	}

	return scroller;

out:
	remove_scroller(scroller);
	return NULL;
}
