#include <stdint.h>
#include "pixel_dodger.h"
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/display/mb_display.h>
#include <zephyr/random/random.h>

static const struct mb_image obstacles[] = {
	MB_IMAGE({0, 0, 0, 0, 0}), MB_IMAGE({1, 0, 1, 0, 1}), MB_IMAGE({1, 1, 0, 1, 1}),
	MB_IMAGE({1, 0, 0, 0, 1}), MB_IMAGE({1, 1, 1, 0, 0}), MB_IMAGE({0, 0, 1, 1, 1}),
	MB_IMAGE({1, 1, 0, 0, 1}), MB_IMAGE({1, 0, 1, 1, 0}), MB_IMAGE({0, 1, 1, 0, 1}),
	MB_IMAGE({1, 0, 0, 1, 1}), MB_IMAGE({1, 1, 0, 0, 1}), MB_IMAGE({1, 0, 1, 1, 1}),
	MB_IMAGE({1, 1, 1, 0, 1}), MB_IMAGE({1, 1, 0, 1, 0}), MB_IMAGE({0, 1, 0, 1, 1}),
	MB_IMAGE({1, 0, 1, 0, 1}), MB_IMAGE({1, 1, 0, 0, 0}), MB_IMAGE({0, 0, 0, 1, 1}),
	MB_IMAGE({1, 0, 0, 1, 0}), MB_IMAGE({0, 1, 0, 0, 1}),
};

int move_left(int player_position)
{
	player_position--; // Decrease position
	printk("Moved left to position %d\n", player_position);
	return player_position; // Return the updated position
}

int move_right(int player_position)
{
	player_position++; // Increase position
	printk("Moved right to position %d\n", player_position);
	return player_position; // Return the updated position
}

const struct mb_image generate_obstacle(void)
{
	int index =
		sys_rand16_get() %
		(sizeof(obstacles) / sizeof(obstacles[0])); // Randomly select an obstacle pattern
	return obstacles[index];
}

void move_obstacles_down(struct mb_image *img)
{
	// Shift each row down by one position
	for (int row = 4; row > 0; row--) {
		img->row[row] = img->row[row - 1];
	}
	// Clear the top row
	img->row[0] = 0;

	printk("Moved obstacles down\n");
}