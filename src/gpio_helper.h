#ifndef _GPIO_HELPER_H

struct gpiod_line_request;

struct gpio_led_desc {
	unsigned int led_id;
	unsigned int pin_id;
	struct gpiod_line_request *gpio_line;
};


int gpio_init();
void gpio_fini();
int gpio_led_request(struct gpio_led_desc *led);
void gpio_led_release(struct gpio_led_desc *led);
int gpio_led_on(struct gpio_led_desc *led);
int gpio_led_off(struct gpio_led_desc *led);

#endif /* _GPIO_HELPER_H */
