#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

/* Helper function definitions */
static void set_board_at(game_state_t *state, unsigned int row,
                         unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t *state, unsigned int snum);
static char next_square(game_state_t *state, unsigned int snum);
static void update_tail(game_state_t *state, unsigned int snum);
static void update_head(game_state_t *state, unsigned int snum);

/* Task 1 */
game_state_t *create_default_state() {
  // TODO: Implement this function.
  unsigned int row = 18, col = 20;
  unsigned int f_row = 2, f_col = 9;
  unsigned int t_row = 2, t_col = 2;
  unsigned int h_row = 2, h_col = 4;

  game_state_t *game = malloc(sizeof(game_state_t));
  game->num_rows = row;
  game->num_snakes = 1;

  // snake:
  game->snakes = malloc(sizeof(snake_t));
  game->snakes->head_col = h_col;
  game->snakes->head_row = h_row;
  game->snakes->tail_col = t_col;
  game->snakes->tail_row = t_row;
  game->snakes->live = true;

  // board:
  game->board = malloc(sizeof(char *) * row);
  for (int i = 0; i < row; i++) {
    (game->board)[i] = malloc(sizeof(char) * (col + 2));
  }
  memset((game->board)[0], '#', col);
  (game->board)[0][col] = '\n';
  (game->board)[0][col + 1] = '\0';
  strcpy((game->board)[row - 1], (game->board)[0]);

  memset((game->board)[1], ' ', col);
  (game->board)[1][0] = (game->board)[1][col - 1] = '#';
  (game->board)[1][col] = '\n';
  (game->board)[1][col + 1] = '\0';

  for (int i = 2; i < row - 1; i++) {
    strcpy((game->board)[i], (game->board)[1]);
  }

  (game->board)[f_row][f_col] = '*';
  (game->board)[h_row][h_col] = 'D';
  (game->board)[t_row][t_col] = 'd';
  (game->board)[t_row][3] = '>';

  return game;
}

/* Task 2 */
void free_state(game_state_t *state) {
  // TODO: Implement this function.
  if (state) {
    if (state->board) {
      for (int i = 0; i < state->num_rows; i++) {
        free((state->board)[i]);
      }
      free(state->board);
    }
    free(state->snakes);
    free(state);
    state = NULL;
  }

  return;
}

/* Task 3 */
void print_board(game_state_t *state, FILE *fp) {
  // TODO: Implement this function.
  for (int i = 0; i < state->num_rows; i++) {
    fprintf(fp, "%s", (state->board)[i]);
  }
  return;
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t *state, char *filename) {
  FILE *f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t *state, unsigned int row, unsigned int col) {
  return state->board[row][col];
}

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t *state, unsigned int row,
                         unsigned int col, char ch) {
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  // TODO: Implement this function.
  return ((c == 'w') || (c == 'a') || ('s' == c) || ('d' == c));
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
  // TODO: Implement this function.
  return (('W' == c) || ('A' == c) || ('S' == c) || ('D' == c) || ('x' == c));
}

static bool is_body(char c) {
  return (('>' == c) || ('<' == c) || ('^' == c) || ('v' == c));
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  // TODO: Implement this function.
  return (is_head(c) || is_tail(c) || is_body(c));
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
  // TODO: Implement this function.
  switch (c) {
  case '>':
    return 'd';
  case '<':
    return 'a';
  case '^':
    return 'w';
  case 'v':
    return 's';
  default:
    return '?';
  }
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
  // TODO: Implement this function.
  switch (c) {
  case 'W':
    return '^';
  case 'A':
    return '<';
  case 'S':
    return 'v';
  case 'D':
    return '>';
  default:
    return '?';
  }
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  // TODO: Implement this function.
  if (('v' == c) || ('s' == c) || ('S' == c)) {
    return cur_row + 1;
  } else if (('w' == c) || ('W' == c) || ('^' == c)) {
    return cur_row - 1;
  } else {
    return cur_row;
  }
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  // TODO: Implement this function.
  if (('>' == c) || ('d' == c) || ('D' == c)) {
    return cur_col + 1;
  } else if (('<' == c) || ('a' == c) || ('A' == c)) {
    return cur_col - 1;
  } else {
    return cur_col;
  }
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake
  is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  if (state && state->num_snakes > snum) {
    const snake_t snake = (state->snakes)[snum];
    const char c = get_board_at(state, snake.head_row, snake.head_col);
    if (snake.live) {
      unsigned int row = get_next_row(snake.head_row, c);
      unsigned int col = get_next_col(snake.head_col, c);
      return get_board_at(state, row, col);
    } else {
      return c;
    }
  }
  return '?';
}

/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the
  head.
*/
static void update_head(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  if (state && state->num_snakes > snum) {
    const snake_t snake = (state->snakes)[snum];
    const char c = get_board_at(state, snake.head_row, snake.head_col);
    if (snake.live) {
      set_board_at(state, snake.head_row, snake.head_col, head_to_body(c));
      unsigned int row = get_next_row(snake.head_row, c);
      unsigned int col = get_next_col(snake.head_col, c);
      set_board_at(state, row, col, c);
      (state->snakes)[snum].head_col = col;
      (state->snakes)[snum].head_row = row;
    }
  }
  return;
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  if (state && state->num_snakes > snum) {
    const snake_t snake = (state->snakes)[snum];
    if (snake.live) {
      char c = get_board_at(state, snake.tail_row, snake.tail_col);
      set_board_at(state, snake.tail_row, snake.tail_col, ' ');
      unsigned int row = get_next_row(snake.tail_row, c);
      unsigned int col = get_next_col(snake.tail_col, c);
      c = get_board_at(state, row, col);
      set_board_at(state, row, col, body_to_tail(c));
      (state->snakes)[snum].tail_col = col;
      (state->snakes)[snum].tail_row = row;
    }
  }
  return;
}

/* Task 4.5 */
void update_state(game_state_t *state, int (*add_food)(game_state_t *state)) {
  // TODO: Implement this function.
  if (state == NULL) {
    return;
  }
  for (unsigned int i = 0; i < state->num_snakes; i++) {
    char c = next_square(state, i);
    if ('#' == c || is_snake(c)) {
      snake_t snake = (state->snakes)[i];
      (state->snakes)[i].live = false;
      set_board_at(state, snake.head_row, snake.head_col, 'x');
    } else if ('*' == c) {
      update_head(state, i);
      add_food(state);
    } else if (' ' == c) {
      update_head(state, i);
      update_tail(state, i);
    }
  }
  return;
}

/* Task 5.1 */
char *read_line(FILE *fp) {
  // TODO: Implement this function.
  char s[100];
  if (fgets(s, 100, fp)) {
    // char *ptr = strchr(s, '\n');
    // unsigned int len = ptr - s + 1;
    char *line = malloc(sizeof(char) * (strlen(s) + 1));
    strcpy(line, s);
    // line[len] = '\0';
    return line;
  } else {
    return NULL;
  }
}

/* Task 5.2 */
game_state_t *load_board(FILE *fp) {
  // TODO: Implement this function.
  unsigned int row = 20, i = 0;
  game_state_t *game = malloc(sizeof(game_state_t));
  game->board = malloc(sizeof(char *) * row);
  for (char *line = read_line(fp); line; i++) {
    if (i >= row) {
      row += 20;
      game->board = realloc(game->board, sizeof(char *) * row);
    }
    (game->board)[i] = malloc(sizeof(char) * (strlen(line) + 1));
    strcpy((game->board)[i], line);
    line = read_line(fp);
  }
  if (0 == i) {
    return NULL;
  } else {
    game->board = realloc(game->board, sizeof(char *) * i);
    game->num_rows = i;
    game->num_snakes = 0;
    game->snakes = NULL;
    return game;
  }
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  return;
}

/* Task 6.2 */
game_state_t *initialize_snakes(game_state_t *state) {
  // TODO: Implement this function.
  return NULL;
}
