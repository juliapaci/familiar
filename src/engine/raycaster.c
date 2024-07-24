#include "raycaster.h"

#include <math.h>
#include <stdbool.h>

const char world_map[MAP_WIDTH][MAP_HEIGHT] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

unsigned char pixels[MAP_WIDTH][MAP_HEIGHT] = {0};

void raycast(Player *player) {
    Pos camera;
    for(unsigned char x = 0; x < MAP_WIDTH; x++) {
        camera.x = (2.0 * ((double) x / MAP_WIDTH) - 1);

        Dir ray = {
            .x = player->dir.x + player->plane.x * camera.x,
            .y = player->dir.y + player->plane.y * camera.x
        };

        Pos map = {
            .x = (int)player->pos.x,
            .y = (int)player->pos.y,
        };

        Vec delta_dist = {
            .x = (ray.x == 0) ? 1e30 : fabs(1 / ray.x),
            .y = (ray.y == 0) ? 1e30 : fabs(1 / ray.y)
        };

        Vec step_len;
        Dir step_dir;
        if(ray.x < 0) {
            step_dir.x = -1;
            step_len.x = (player->pos.x - map.x) * delta_dist.x;
        } else {
            step_dir.x = 1;
            step_len.x = (map.x + 1.0 - player->pos.x) * delta_dist.x;
        }
        if (ray.y < 0) {
            step_dir.y = -1;
            step_len.y = (player->pos.y - map.y) * delta_dist.y;
        } else {
            step_dir.y = 1;
            step_len.y = (map.y + 1.0 - player->pos.y) * delta_dist.y;
        }

        Hit side;
        // DDA
        while(world_map[(int)map.x][(int)map.y] > 0) {
            if(step_len.x < step_len.y) {
                step_len.x += delta_dist.x;
                map.x += step_dir.x;
                side = NS;
            } else {
                step_len.y += delta_dist.y;
                map.y += step_dir.y;
                side = EW;
            }
        }

        const double perp_dist = (side == NS) ? (step_len.x - delta_dist.x) : (step_len.y - delta_dist.y);
        const int h = MAP_HEIGHT / perp_dist;
        unsigned char height = h / perp_dist;
        unsigned char base = fmax(-height / 2.0 + h / 2.0, 0);
        unsigned char top = fmin(height / 2.0 + h / 2.0, MAP_HEIGHT - 1);

        for(unsigned char y = base; y < top; y++) {
            pixels[x][y] = 255;
        }
    }
}
