// https://lodev.org/cgtutor/raycasting.html

#ifndef __RAYCASTER_H__
#define __RAYCASTER_H__

#define MAP_WIDTH 24
#define MAP_HEIGHT 24

extern unsigned char pixels[MAP_WIDTH][MAP_HEIGHT];

typedef struct {
    double x;
    double y;
} Vec;
typedef Vec Pos;
typedef Vec Dir;

typedef struct {
    Pos pos;
    Dir dir;
    Pos plane; // camera plane
} Player;

typedef enum {
    NS, EW
} Hit;

// raycasting loop
void raycast(Player *player);

#endif // __RAYCASTER_H__
