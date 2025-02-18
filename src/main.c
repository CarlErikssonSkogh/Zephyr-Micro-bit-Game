#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/display/mb_display.h>
#include <zephyr/drivers/gpio.h>
#include "pixel_dodger.h"

#define BASKET_ROW 4 /* Real Y coordinate of the paddle */

// Define a semaphore
static K_SEM_DEFINE(disp_update, 0, 1);

// Define the buttons
static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0});
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw1), gpios, {0});

// Variable to store the player's position locally
static int player_position = 0;

// Timer to control obstacle movement
static struct k_timer obstacle_timer;
static struct mb_image img = {};
static int row_counter = 0;

static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	if (pins & BIT(button0.pin)) {
		player_position = move_left(player_position);
		printk("New position after moving left: %d\n", player_position);
	}

	if (pins & BIT(button1.pin)) {
		player_position = move_right(player_position);
		printk("New position after moving right: %d\n", player_position);
	}
}

static void configure_buttons(void)
{
	// Define a variable with the gpio_callback structure
	static struct gpio_callback button_cb_data;

	// Check if buttons device is ready (button0.port = button1.port)
	if (!gpio_is_ready_dt(&button0)) {
		printk("Error: button0 device %s is not ready\n", button0.port->name);
		return;
	}
	// Configure button0 pin as input
	if (gpio_pin_configure_dt(&button0, GPIO_INPUT) != 0) {
		printk("Error: failed to configure %s pin %d\n", button0.port->name, button0.pin);
		return;
	}

	// Configure button1 pin as inputw
	if (gpio_pin_configure_dt(&button1, GPIO_INPUT) != 0) {
		printk("Error: failed to configure %s pin %d\n", button1.port->name, button1.pin);
		return;
	}

	// Configure interrupts for button0 and button1
	// This sets up the pins to generate an interrupt on the rising edge (button press)
	gpio_pin_interrupt_configure_dt(&button0, GPIO_INT_EDGE_TO_ACTIVE);
	gpio_pin_interrupt_configure_dt(&button1, GPIO_INT_EDGE_TO_ACTIVE);

	// Initialize the callback structure for both buttons
	// This sets up the button_cb_data structure with the button_pressed function
	// and specifies that the callback should be triggered by button0 and button1 pins
	gpio_init_callback(&button_cb_data, button_pressed, BIT(button0.pin) | BIT(button1.pin));

	// Add the callback to the GPIO port
	// This registers the callback with the GPIO driver so that the button_pressed
	// function will be called when an interrupt occurs on the specified pins
	gpio_add_callback(button0.port, &button_cb_data);
}

static void obstacle_timer_handler(struct k_timer *dummy)
{
	move_obstacles_down(&img);

	if (row_counter % 4 == 0) {
		struct mb_image obstacle = generate_obstacle();
		img.row[0] = obstacle.row[0];
	} else {
		img.row[0] = 0;
	}

	row_counter++;
}

int main(void)
{
	struct mb_display *disp = mb_display_get();

	if (!disp) {
		printk("Failed to get display device\n");
		return -1;
	}

	/* Display countdown from '1' to '3' */
	mb_display_print(disp, MB_DISPLAY_MODE_SINGLE, 1 * MSEC_PER_SEC, "123");
	k_sleep(K_SECONDS(4));

	// Initialize the obstacle timer to call obstacle_timer_handler every second
	k_timer_init(&obstacle_timer, obstacle_timer_handler, NULL);
	k_timer_start(&obstacle_timer, K_SECONDS(1), K_SECONDS(1));

	while (1) {
		// Configure the buttons
		configure_buttons();

		// Prepare the display update
		img.row[BASKET_ROW] = BIT(player_position);

		// Update the display
		mb_display_image(disp, MB_DISPLAY_MODE_SINGLE, SYS_FOREVER_MS, &img, 1);

		// Add a delay to achieve 30 fps
		k_sleep(K_MSEC(33)); // 33 milliseconds delay
	}
	return 0;
}