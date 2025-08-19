#include <assert.h>
#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
  int GRID_HEIGHT;
  int GRID_WIDTH;
  int NUM_BOMBS;

  int** grid;
  int** visual_grid;

  Texture2D textures[13];

  int bombs;
  bool game_running;
  bool won;
} game;


void populate_grid(game* game) {
  int i, j;
  for(game->bombs = 0; game->bombs < game->NUM_BOMBS; game->bombs++) {
    again:
    i = rand() % game->GRID_HEIGHT;
    j = rand() % game->GRID_WIDTH;
    if(game->visual_grid[i][j]) goto again;
    game->visual_grid[i][j] = 1;
  }

  for(int i = 0; i < game->GRID_HEIGHT; i++) {
    for(int j = 0; j < game->GRID_WIDTH; j++) {
      if(game->visual_grid[i][j]) {
        game-> grid[i][j] = 10;
        continue;
      }
      int b = 0;
      if(i) {
        if(j)     b += game->visual_grid[i - 1][j - 1]; // top left
                  b += game->visual_grid[i - 1][j];     // top
        if(j < game->GRID_WIDTH - 1) b += game->visual_grid[i - 1][j + 1]; // top right
      }
      if(j)     b += game->visual_grid[i][j - 1];       // left
      if(j < game->GRID_WIDTH - 1) b += game->visual_grid[i][j + 1];       // right
      if(i < game->GRID_HEIGHT - 1) {
        if(j)     b += game->visual_grid[i + 1][j - 1]; // bottom left
                  b += game->visual_grid[i + 1][j];     // bottom
        if(j < game->GRID_WIDTH - 1) b += game->visual_grid[i + 1][j + 1]; // bottom right
      }
      game-> grid[i][j] = b;
    }
  }

  for(int i = 0; i < game->GRID_HEIGHT; i++) {
    for(int j = 0; j < game->GRID_WIDTH; j++) {
      game->visual_grid[i][j] = 9;
    }
  }
}

void load_textures(game* game) {
  char tmp[13];
  for(int i = 0; i <= 8; i++) {
    sprintf(tmp, "assets/%d.png", i);
    game->textures[i] = LoadTexture(tmp);
  }
  game->textures[9] = LoadTexture("assets/unexplored.png");
  game->textures[10] = LoadTexture("assets/mine.png");
  game->textures[11] = LoadTexture("assets/mine_hit.png");
  game->textures[12] = LoadTexture("assets/flag.png");
}

void draw_grid(game* game) {
  for(int i = 0; i < game->GRID_HEIGHT; i++) {
    for(int j = 0; j < game->GRID_WIDTH; j++) {
      DrawTexture(game->textures[game->visual_grid[i][j]], j * 21 + 10, i * 21 + 30, WHITE);
    }
  }
}

void clear_all(game* game, int noti, int notj) {
  for(int i = 0; i < game->GRID_HEIGHT; i++) {
    for(int j = 0; j < game->GRID_WIDTH; j++) {
      if(i == noti && j == notj) continue;
      if(game->visual_grid[i][j] == 12) continue;
      game->visual_grid[i][j] = game-> grid[i][j];
    }
  }
}

void end_game(game* game, int i, int j) {
  game->game_running = false;
  clear_all(game, i, j);
}

void update_status(game* game, int i, int j, bool r) {
  if(i < 0 || i >= game->GRID_HEIGHT || j < 0 || j >= game->GRID_WIDTH) return;
  if(game->visual_grid[i][j] == 12 || game->visual_grid[i][j] == 0) return;
  else if(1 <= game->visual_grid[i][j] && game->visual_grid[i][j] <= 8 && !r) {
    update_status(game, i - 1, j - 1, 1);
    update_status(game, i - 1, j, 1);
    update_status(game, i - 1, j + 1, 1);
    update_status(game, i, j - 1, 1);
    update_status(game, i, j + 1, 1);
    update_status(game, i + 1, j - 1, 1);
    update_status(game, i + 1, j, 1);
    update_status(game, i + 1, j + 1, 1);
    return;
  }
  game->visual_grid[i][j] = game->grid[i][j] == 10 ? 11 : game->grid[i][j];
  if(game->visual_grid[i][j] == 11) end_game(game, i, j);
  else if(!game->visual_grid[i][j]) {
    update_status(game, i - 1, j - 1, 1);
    update_status(game, i - 1, j, 1);
    update_status(game, i - 1, j + 1, 1);
    update_status(game, i, j - 1, 1);
    update_status(game, i, j + 1, 1);
    update_status(game, i + 1, j - 1, 1);
    update_status(game, i + 1, j, 1);
    update_status(game, i + 1, j + 1, 1);
  } 
}

void toggle_flag(game* game, int i, int j) {
  if(i < 0 || i >= game->GRID_HEIGHT || j < 0 || j >= game->GRID_WIDTH) return;
  if(game->visual_grid[i][j] == 12) {game->visual_grid[i][j] = 9; game->bombs++; return;}
  if(game->visual_grid[i][j] != 9 || game->bombs == 0) return;
  game->visual_grid[i][j] = 12;
  game->bombs--;
}

void check_win(game* game) {
  for(int i = 0; i < game->GRID_HEIGHT; i++) {
    for(int j = 0; j < game->GRID_WIDTH; j++) {
      if(game->visual_grid[i][j] == 9) return;
      if(game->visual_grid[i][j] == 12 && game-> grid[i][j] != 10) return;
    }
  }
  game->game_running = false;
  game->won = true;
}

void init_game(game* game, int mode) {
  game->GRID_HEIGHT = mode ? 16 : 8;
  game->GRID_WIDTH = mode ? (mode == 2 ? 30 : 16) : 8;
  game->NUM_BOMBS = mode ? (mode == 2 ? 99 : 40) : 10;

  game->grid = malloc(sizeof(int*) * game->GRID_HEIGHT);
  game->visual_grid = malloc(sizeof(int*) * game->GRID_HEIGHT);
  for(int i = 0; i < game->GRID_HEIGHT; i++) {
    game->grid[i] = calloc(game->GRID_WIDTH, sizeof(int));
    game->visual_grid[i] = calloc(game->GRID_WIDTH, sizeof(int));
  }

  game->game_running = true;
}

int main(int argc, char** argv) {
  srand(getpid());
  game game = {0};

  int mode = 0;
  if(argc > 1) {
    if(strcmp("beginner", argv[1]) == 0) {}
    else if(strcmp("intermediate", argv[1]) == 0) mode = 1;
    else if(strcmp("expert", argv[1]) == 0) mode = 2;
    else printf("expected one of 'beginner', 'intermediate', 'expert'\n");
  }

  init_game(&game, mode);

  populate_grid(&game);

  InitWindow(game.GRID_WIDTH * 21 + 20, game.GRID_HEIGHT * 21 + 40, "raysweeper");

  load_textures(&game);

  SetTargetFPS(60);

  while(!WindowShouldClose()) {
    if(game.game_running) {
      if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        update_status(&game, (GetMouseY() - 10) / 21 - 1, (GetMouseX() - 10) / 21, 0);
      }
      if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        toggle_flag(&game, (GetMouseY() - 10) / 21 - 1, (GetMouseX() - 10) / 21);
      }

      if(!game.bombs) check_win(&game);
    }

    char bomb_num[4];
    char time[4];
    BeginDrawing();
    {
      ClearBackground(GRAY);
      if(game.game_running) {
        sprintf(bomb_num, "%.03d", game.bombs);
        sprintf(time, "%.03d", (int) GetTime());
      }
      DrawText(bomb_num, 10, 10, 20, RED);
      DrawText(time, (game.GRID_WIDTH - 1) * 21 - 5, 10, 20, RED);
      if(game.won) DrawText("YOU WON", (game.GRID_WIDTH - 3) / 2 * 21 + 2, 10, 20, RED);
      draw_grid(&game);
    }
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
