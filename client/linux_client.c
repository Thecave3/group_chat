#include "../libs/common.h"

#define DEFAULT_NAME "thecave3"
#define MAX_LEN_NAME 8

#define DELAY 3000 // cancellare a tempo debito

void draw_borders(WINDOW *screen) {
  int x, y, i;

  getmaxyx(screen, y, x);

  // 4 corners
  mvwprintw(screen, 0, 0, "+");
  mvwprintw(screen, y - 1, 0, "+");
  mvwprintw(screen, 0, x - 1, "+");
  mvwprintw(screen, y - 1, x - 1, "+");

  // sides
  for (i = 1; i < (y - 1); i++) {
    mvwprintw(screen, i, 0, "|");
    mvwprintw(screen, i, x - 1, "|");
  }

  // top and bottom
  for (i = 1; i < (x - 1); i++) {
    mvwprintw(screen, 0, i, "-");
    mvwprintw(screen, y - 1, i, "-");
  }
}

void screen_setup() {
  int parent_x, parent_y, new_x, new_y;
  int score_size = 3;

  initscr();
  noecho();
  curs_set(FALSE);

  // set up initial windows
  getmaxyx(stdscr, parent_y, parent_x);

  WINDOW *field = newwin(parent_y - score_size, parent_x, 0, 0);
  WINDOW *score = newwin(score_size, parent_x, parent_y - score_size, 0);

  draw_borders(field);
  draw_borders(score);

  while(1) {
    getmaxyx(stdscr, new_y, new_x);

    if (new_y != parent_y || new_x != parent_x) {
      parent_x = new_x;
      parent_y = new_y;

      wresize(field, new_y - score_size, new_x);
      wresize(score, score_size, new_x);
      mvwin(score, new_y - score_size, 0);

      wclear(stdscr);
      wclear(field);
      wclear(score);

      draw_borders(field);
      draw_borders(score);
    }

    // draw to our windows
    mvwprintw(field, 1, 1, "Field");
    mvwprintw(score, 1, 1, "Score");

    // refresh each window
    wrefresh(field);
    wrefresh(score);
  }

  endwin();

}

int main(int argc, char *argv[]) {
  int ret;
  int sock;
  struct sockaddr_in server_addr = {0};
  char* name;

  if (argv[1] != NULL)
      name = argv[1];
  else
      name = DEFAULT_NAME;

  printf("Your name is %s\n", name);

  sock = socket(AF_INET,SOCK_STREAM,0);
  ERROR_HELPER(sock,"Errore creazione socket: ");

  server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
  ret = connect(sock,(struct sockaddr*) &server_addr,sizeof(struct sockaddr_in));
  ERROR_HELPER(ret,"Errore connect: ");



  return 0;
}
