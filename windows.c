#include "engine.h"

struct widget {
  void (*get_dimensions)(void *data, int *w, int *h, int *min_w, int *min_h);
  void (*set_dimensions)(void *data, int w, int h);
};

struct layout_widget_list {
  struct widget_list *next;
  struct widget *widget;
};

#define LAYOUT_VBOX 1
#define LAYOUT_HBOX 2
struct layout {
  int type;
  int spacing;
  struct layout_widget_list *widgets;
  /* union with additional parameter */
};

struct window {
  int x;
  int y;
  int width;
  int height;
};


void window_get_dimensions(struct window* win, int *w, int *h);
void widget_set_dimensions(struct widget* wgt, int w, int h);
void widget_get_dimensions(struct widget* wgt, int *w, int *h, int *minw, int *minh);

void window_set_dimensions(struct window* win, int w, int h) {
  win->width = w;
  win->height = h;
}
void window_set_position(struct window *win, int x, int y)
{
  win->x = x;
  win->y = y;
}
void widget_draw(struct widget *wgt, int x, int y);
void window_draw(struct window *win)
{
  draw_color(255,255,255,255);
  draw_rect4(win->x,win->y,win->width,win->height);
}

static void init(void **data)
{
  *data = NULL;
}

static void update(void *data, float delta)
{
}

static void draw(void *data)
{
  struct window test_win;
  window_set_dimensions(&test_win, 100, 100);
  window_set_position(&test_win, 10, 10);
  draw_color(0,0,0,0);
  clear_screen();
  window_draw(&test_win);
}

static struct game_ctx ctx = {
  .screen_width = 320,
  .screen_height = 240,
  .screen_scale = 1,
  .game_name = "window-test",
  .game_load = NULL, //(void **data);
  .game_init = init, //)(void **data);
  .game_update = update, //)(void *data, float delta);
  .game_draw = draw, //)(void *data);
  .game_on_key_down = NULL, //)(int key, void *data);
  .game_on_key_up = NULL, //)(int key, void *data);
  .game_text_input = NULL, //)(char *text, void* data);
  .game_on_quit = NULL, //)(void (*void (*void (*void *data);
  .game_on_mouse_down = NULL, //)(int x, int y, int button, void *data);
  .game_on_mouse_up = NULL, //)(int x, int y, int button, void *data);
  .game_on_mouse_motion = NULL, //)(int x, int y, void *data);
};

ENGINE_MAIN(&ctx);
