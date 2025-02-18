#include <zephyr/display/mb_display.h>

// Move the catcher left and return the updated position
int move_left(int player_position);

// Move the catcher right and return the updated position
int move_right(int player_position);

// Generate the obstacles
const struct mb_image generate_obstacle(void);

// Move obstacles down the screen
void move_obstacles_down(struct mb_image *img);
