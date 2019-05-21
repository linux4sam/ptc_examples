/*
 * Demo for PTC QT6 wing.
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <libevdev-1.0/libevdev/libevdev.h>

#include "ptc_qt.h"

#define SLIDER_X_INPUT_FILE	"/dev/input/atmel_ptc0"
#define SLIDER_Y_INPUT_FILE	"/dev/input/atmel_ptc1"
#define POLL_NFDS		2

static void slider_position_update(struct led_desc *leds, unsigned int nleds,
				    unsigned int ev_type, unsigned int ev_value,
				    void *arg)
{
	unsigned int *position = arg;

	if (ev_type == EV_KEY)
		if (ev_value == 0)
			*position = 0;

	if (ev_type == EV_ABS)
		*position = ev_value;
}

int main(void)
{
	struct scroller *slider_x, *slider_y;
	int pos_x = 0, pos_y = 0, ret, i;
	struct pollfd fds[POLL_NFDS];

	slider_x = initialize_scroller(SLIDER_X_INPUT_FILE, NULL, 0, NULL,
				       slider_position_update);
	if (!slider_x)
		goto out;

	slider_y = initialize_scroller(SLIDER_Y_INPUT_FILE, NULL, 0, NULL,
				       slider_position_update);
	if (!slider_y)
		goto slider_y_fail;

	fds[0].fd = slider_x->fd;
	fds[0].events = POLLIN;
	fds[1].fd = slider_y->fd;
	fds[1].events = POLLIN;

	printf("demo running...\n");
	while (1) {
		ret = poll(fds, POLL_NFDS, - 1);
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

			if (fds[i].fd == slider_x->fd) {
				ret = scroller_event_handler(slider_x, &pos_x);
				if (ret)
					break;
			}

			if (fds[i].fd == slider_y->fd) {
				ret = scroller_event_handler(slider_y, &pos_y);
				if (ret)
					break;
			}
		}

		printf("x=%d - y=%d\n", pos_x, pos_y);
	}
	fprintf(stderr, "event error\n");

	remove_scroller(slider_y);
slider_y_fail:
	remove_scroller(slider_x);
out:
	return EXIT_FAILURE;
}
