#include "snake_map.h"
#include <pthread.h>
#include <iostream>
#include <vector>
#include <utility>
#include <stdlib.h>
#include <time.h>
#include "macros.h"

using namespace std;

SnakeMap::SnakeMap(Snake *snake)
{
    this->snake = snake;
    clear_map(this->map_array);
    srand(time(NULL));
    update_snake_food(true);

    // initialize map nodes
    for (int x = 0; x < MAP_HEIGHT; x++) {
        for (int y = 0; y < MAP_WIDTH; y++) {
            map_nodes[x][y].coord.x = x;
            map_nodes[x][y].coord.y = y;
            map_nodes[x][y].dist = -1;
            map_nodes[x][y].legal = true;
            map_nodes[x][y].shortest_prev = nullptr;
            map_nodes[x][y].neighborCount = 0;
            if (x > 0) map_nodes[x][y].neighbors[map_nodes[x][y].neighborCount++] = &map_nodes[x-1][y];
            if (x < MAP_HEIGHT-1) map_nodes[x][y].neighbors[map_nodes[x][y].neighborCount++] = &map_nodes[x+1][y];
            if (y > 0) map_nodes[x][y].neighbors[map_nodes[x][y].neighborCount++] = &map_nodes[x][y-1];
            if (y < MAP_WIDTH-1) map_nodes[x][y].neighbors[map_nodes[x][y].neighborCount++] = &map_nodes[x][y+1];
        }
    }
}

void SnakeMap::redraw(void)
{
    clear_map(this->map_array);
    for (int i = 0; i < MAP_END; i++)
    {
        cout << endl;
    }
    update_score();
    vector<pair<int, int>> snake_parts = snake->snake_parts;
    for (int i = 0; i < snake_parts.size(); i++)
    {
        pair<int, int> tmp = snake_parts[i];
        map_array[tmp.first][tmp.second] = SNAKE_CHAR;
    }
    update_snake_head(map_array, snake);
    update_snake_food(false);
    map_array[snake_food.first][snake_food.second] = SNAKE_FOOD_CHAR;
    if (snake->Help_win_mode) {
        draw_green_line();
    }
    snake->Help_win_mode = snake->Help_win_mode_next;

    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        for (int j = 0; j < MAP_WIDTH; j++)
        {
            if (map_array[i][j] == MAP_HINT) {
                cout << "\033[32m"; // Set green color
            }
            cout << map_array[i][j] << "\033[0m" << " ";
        }
        cout << endl;
    }
}

void SnakeMap::update_snake_food(bool force_update)
{
    if (snake->food_eaten || force_update)
    {
        while (true)
        {
            int random_i = rand() % MAP_WIDTH;
            int random_j = rand() % MAP_HEIGHT;
            if (map_array[random_i][random_j] == MAP_CHAR)
            {
                snake_food = make_pair(random_i, random_j);
                snake->set_snake_food(snake_food);
                snake->food_eaten = false;
                break;
            }
        }
    }
}

void clear_map(char map_array[MAP_HEIGHT][MAP_WIDTH])
{
    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        for (int j = 0; j < MAP_WIDTH; j++)
        {
            map_array[i][j] = MAP_CHAR;
        }
    }
}

void update_snake_head(char map_array[MAP_HEIGHT][MAP_WIDTH], Snake *snake)
{
    char snake_head_char = SNAKE_CHAR;
    enum Direction direction = snake->get_direction();
    switch (direction)
    {
    case West:
        snake_head_char = SNAKE_HEAD_WEST;
        break;
    case North:
        snake_head_char = SNAKE_HEAD_NORTH;
        break;
    case East:
        snake_head_char = SNAKE_HEAD_EAST;
        break;
    case South:
        snake_head_char = SNAKE_HEAD_SOUTH;
        break;
    }
    pair<int, int> snake_head = snake->snake_head;
    map_array[snake_head.first][snake_head.second] = snake_head_char;
}

void SnakeMap::update_score(void)
{
    cout << "Score:" << snake->length << endl;
}

int SnakeMap::traverse_map_node(PathNode* node, PathNode* prev, int dist) {
    // Skips if node is nullptr, already has a shorter or equal path from start, or is illegal (snake body/wall collision)
    if (node == nullptr || (node->dist != -1 && node->dist <= dist) || !node->legal) {
        return -1;
    }
    node->dist = dist;
    node->shortest_prev = prev;
    Coord cur = node->coord;
    // Stopping condition: if we reached food, return distance. Otherwise, continue traversing neighbors.
    if (map_array[cur.x][cur.y] == SNAKE_FOOD_CHAR)
        return dist;
    int min_dist = -1;
    PathNode* min_dist_neighbor = nullptr;
    for (int i = 0; i < node->neighborCount; i++) {
        PathNode* neighbor = node->neighbors[i];
        int neighbor_dist = traverse_map_node(neighbor, node, dist+1);
        if (neighbor_dist != -1 && (min_dist == -1 || neighbor_dist < min_dist)) {
            min_dist = neighbor_dist;
            min_dist_neighbor = neighbor;
        }
    }
    return min_dist;
}

void SnakeMap::find_possible_route(Coord hint_array[MAP_HEIGHT * MAP_WIDTH - 1], int *hint_len, Coord food) {
    // BFS from head to food
    Coord head {snake->snake_head.first, snake->snake_head.second};
    PathNode* start = &map_nodes[head.x][head.y];
    start->dist = 0;
    int min_dist = -1;
    PathNode* min_dist_neighbor = nullptr;
    for (int i = 0; i < start->neighborCount; i++) {
        PathNode* neighbor = start->neighbors[i];
        int dist = traverse_map_node(neighbor, start, start->dist + 1);
        if (dist != -1 && (min_dist == -1 || dist < min_dist)) {
            min_dist = dist;
            min_dist_neighbor = neighbor;
        }
    }

    // backtrack from food to head to get hint path
    PathNode* cur =  &map_nodes[food.x][food.y];
    while (cur != nullptr && (cur->coord.x != head.x || cur->coord.y != head.y)) {
        hint_array[*hint_len] = cur->coord;
        *hint_len += 1;
        cur = cur->shortest_prev;
    }
}

void SnakeMap::draw_green_line(void) {
    Coord hint_array[MAP_HEIGHT * MAP_WIDTH - 1];
    Coord food;
    int hint_len = 0;
    // initialize distance, legal and prev ptr on map nodes for new BFS run and find food coordinates on map
    for (int x = 0; x < MAP_HEIGHT; x++) {
        for (int y = 0; y < MAP_WIDTH; y++) {
            map_nodes[x][y].dist = -1;
            map_nodes[x][y].legal = (map_array[x][y] == MAP_CHAR || map_array[x][y] == SNAKE_FOOD_CHAR);
            map_nodes[x][y].shortest_prev = nullptr;
            if (map_array[x][y] == SNAKE_FOOD_CHAR) {
                food = {x, y};
            }
        }
    }
    find_possible_route(hint_array, &hint_len, food);
    // from 1 to skip food coordinate.
    for (int i = 1; i < hint_len; i++) {
        Coord cur = hint_array[i];
        map_array[cur.x][cur.y] = MAP_HINT;
    }
}