/*
 * Demo for ATQT2 wing.
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

#include <linux/i2c-dev.h>

#include <libevdev-1.0/libevdev/libevdev.h>

#include "atqt.h"

#define SLIDER_X_INPUT_FILE	"/dev/input/event1"
#define SLIDER_Y_INPUT_FILE	"/dev/input/event2"
#define POLL_NFDS		2

#define IS31FL3728_ADDR			0x60
#define IS31FL3728_UPDATE_COLUMN_REG	0xc
#define I2C_DEVICE_FILE			"/dev/i2c-1"

int i2c_fd;
char col_state[7] = {0};

static void led_update()
{
	char buf[2];

	buf[0] = IS31FL3728_UPDATE_COLUMN_REG;
	buf[1] = 0x1;
	if (write(i2c_fd, buf, 2) != 2) {
		fprintf(stderr, "Failed to write to the i2c bus\n");
		exit(EXIT_FAILURE);
	}
}

static void led_all_off(void)
{
	char buf[2];
	int i;

	buf[1] = 0;
	for (i = 0; i < 8; i++) {
		buf[0] = i;
		if (write(i2c_fd, buf, 2) != 2) {
			fprintf(stderr, "Failed to write to the i2c bus\n");
			exit(EXIT_FAILURE);
		}
	}

	led_update();
}

static void led_on(int xpos, int ypos)
{
	char buf[2];

	led_all_off();

	if (!xpos && !ypos)
		return;

	/* xpos: 0 to 63, ypos: 0 to 57 */
	buf[0] = 1 + xpos / 10;
	buf[1] = 0b01000000 >> (ypos / 9);

	if (write(i2c_fd, buf, 2) != 2) {
		fprintf(stderr, "Failed to write to the i2c bus\n");
		exit(EXIT_FAILURE);
	}

	led_update();
}

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
	int pos_x = 0, pos_y = 0, i, ret;
	struct pollfd fds[POLL_NFDS];

	if ((i2c_fd = open(I2C_DEVICE_FILE, O_RDWR)) < 0) {
		fprintf(stderr, "Can't open %s\n", I2C_DEVICE_FILE);
		return EXIT_FAILURE;
	}

	if (ioctl(i2c_fd, I2C_SLAVE, IS31FL3728_ADDR) < 0) {
		fprintf(stderr, "Failed to acquire bus access and/or talk to slave\n");
		goto out;
	}

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

		led_on(pos_x, pos_y);
	}
	fprintf(stderr, "event error\n");

	remove_scroller(slider_y);
slider_y_fail:
	remove_scroller(slider_x);
out:
	close(i2c_fd);
	return EXIT_FAILURE;
}
