/*
 * Library for ATQTx demos.
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

#ifndef _ATQT_H
#define _ATQT_H

#include <libevdev-1.0/libevdev/libevdev.h>
#include <gpiod.h>

struct led_desc {
	unsigned int led_id;
	unsigned int pin_id;
	struct gpiod_line *gpio_line;
};

struct buttons {
	int fd;
	struct libevdev *evdev;
	unsigned int *key_codes;
	struct led_desc *leds;
};

struct scroller {
	int fd;
	struct libevdev *evdev;
	struct led_desc *leds;
	unsigned int nleds;
	void (*position_update)(struct led_desc *leds, unsigned int nleds,
				unsigned int ev_type, unsigned int ev_value,
				void *arg);
};

int scroller_event_handler(struct scroller *scroller, void *arg);
struct scroller *initialize_scroller(const char *input_file,
	struct led_desc *leds, unsigned int nleds,
	struct gpiod_chip *gpiochip,
	void (*position_update)(struct led_desc *leds, unsigned int nleds,
				unsigned int ev_type, unsigned int ev_value,
				void *arg)
	);
void remove_scroller(struct scroller *scroller);

#endif /* _ATQT_H */
