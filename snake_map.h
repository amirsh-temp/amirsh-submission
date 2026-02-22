#ifndef _snake_map_h
#define _snake_map_h

#include "snake.h"
#include "macros.h"

struct Coord {
    int x;
    int y;
};

struct PathNode {
    Coord coord; // (x,y) coordinate of this node
    int dist; // shortest number of steps from start; -1 = unknown
    PathNode* shortest_prev; // previous node on shortest path
    bool legal; // whether this coordinate is legal (no collision)
    PathNode* neighbors[4]; // up to 4 neighbors (N,E,S,W) (nullptr if no neighbor)
    int neighborCount; // number of neighbors
};

class SnakeMap
{
public:
  SnakeMap(Snake *snake);
  void redraw();
  pair<int, int> snake_food;
  void update_snake_food(bool force_update);
  void update_score();

private:
  char map_array[MAP_HEIGHT][MAP_WIDTH];
  PathNode map_nodes[MAP_HEIGHT][MAP_WIDTH];
  Snake *snake;
  int traverse_map_node(PathNode* node, PathNode* prev, int dist);
  void find_possible_route(Coord hint_array[MAP_HEIGHT * MAP_WIDTH - 1], int *hint_len, Coord food);
  void draw_green_line();
};

void clear_map(char map_array[MAP_HEIGHT][MAP_WIDTH]);
void update_snake_head(char map_array[MAP_HEIGHT][MAP_WIDTH], Snake *snake);

#endif