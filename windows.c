#include "engine.h"

enum object_type_e {OBJECT_T_WINDOW, OBJECT_T_WIDGET, OBJECT_T_LAYOUT};

struct win_object {
  enum object_type_e type;
  void (*get_dimensions)(void *data, int *w, int *h);
  void (*set_dimensions)(void *data, int w, int h);
  void (*set_position)(void *data, int x, int y);
  void (*get_position)(void *data, int *x, int *y);
  void (*draw)(void *data, int x, int y);
  SDL_Rect rect;
};

static void object_set_dimensions_cb(void *object, int w, int h)
{
  struct win_object *ctx = object;
  ctx->rect.w = w;
  ctx->rect.h = h;
}

static void object_set_position_cb(void *object, int x, int y)
{
  struct win_object *ctx = object;
  ctx->rect.x = x;
  ctx->rect.y = y;
}

static void object_get_position_cb(void *object, int *x, int *y)
{
  struct win_object *ctx = object;
  *x = ctx->rect.x;
  *y = ctx->rect.y;
}

static void object_get_dimensions_cb(void *object, int *w, int *h)
{
  struct win_object *ctx = object;
  *w = ctx->rect.w;
  *h = ctx->rect.h;
}

void *object_new(enum object_type_e type, int size)
{
  struct win_object *ret = calloc(1, size);
  ret->type = type;
  ret->set_dimensions = object_set_dimensions_cb;
  ret->get_dimensions = object_get_dimensions_cb;
  ret->set_position = object_set_position_cb;
  ret->get_position = object_get_position_cb;
  return ret;
}

void object_get_dimensions(void *object, int *w, int *h)
{
  struct win_object *ctx = object;
  ctx->get_dimensions(object, w, h);
}

void object_set_dimensions(void *object, int w, int h)
{
  struct win_object *ctx = object;
  ctx->set_dimensions(object, w, h);
}

void object_set_position(void *object, int w, int h)
{
  struct win_object *ctx = object;
  ctx->set_position(object, w, h);
}

void object_get_position(void *object, int *w, int *h)
{
  struct win_object *ctx = object;
  ctx->get_position(object, w, h);
}

void object_draw(void *object, int x, int y)
{
  struct win_object *ctx = object;
  ctx->draw(object, x, y);
}


struct widget {
  struct win_object object;
};

/* label widget */
struct widget_label {
  struct widget widget;
  char *text;
  int font;
};

void label_get_dimensions(void *object, int *w, int *h)
{
  struct widget_label *ctx = object;
  SDL_Rect rect;
  text_dimensions(ctx->text, &rect);
  *w = rect.w;
  *h = rect.h;
}

struct widget_label *new_label(char *text, int font)
{
  struct widget_label *ret = object_new(OBJECT_T_WIDGET, sizeof(*ret));
  ret->text = strdup(text);
  ret->font = FONT_DEFAULT;
  return ret;
}

struct layout_widget_list {
  struct widget_list *next;
  struct widget *widget;
};

enum layout_type_e {LAYOUT_T_HBOX, LAYOUT_T_VBOX, LAYOUT_T_TABLE};
struct layout {
  struct win_object object;
  struct object *objects;
  int object_count;
  void (*add)(struct layout*, void *object, const char *flags);
};

void layout_add(struct layout *layout, void *object, const char *flags)
{
  layout->add(layout, object, flags);
}

void vbox_set_dimensions(void *object, int w, int h)
{
  //struct layout *ctx = object;
  object_set_dimensions_cb(object, w, h);
  /* calculate the average size */
  /* to do this 
   *  1 find all objects that have the shrink-flag
   *  2 get the minimum size from all shrinked objects
   *  3 decrease <width> by the sum of the size of all shrinked objects
   *    and mark object as 'done'
   *  4 divide <with> by the number of expanded-flagged objects
   *    to get the average size
   *  5 get minimum with for each expanded-flagged object
   *    if the minimum size exceeds the average size
   *      decrease it's minimum size from <width>, mark object as 'done' then
   *      set new_average to  <with> divided by the count of remaining
   *      expanded-flagged objects
   *      if this is the last object
   *        increase <width> to match content <width>
   *  6 set all remaining (not 'done') objects to the average-size */
}

struct layout *new_vbox(void)
{
  struct layout *ret = calloc(1, sizeof(*ret));
  ret->object.type = OBJECT_T_LAYOUT;
  ret->object.set_dimensions = vbox_set_dimensions;
  ret->add = layout_add;
  return ret;
};

struct window {
  struct win_object object;
  struct layout *layout;
};

static void window_set_dimensions_cb(void* object, int w, int h) {
  struct window *ctx = object;
  /* call parent function */
  object_set_dimensions_cb(object, w, h);
  if (ctx->layout) {
    object_set_dimensions(ctx->layout, w, h);
  }
}

void window_draw(void *data, int x, int y)
{
  struct window *win = data;
  draw_color(255,255,255,255);
  draw_rect(&win->object.rect);
}

struct window *window_new(int w, int h)
{
  struct window *ret = object_new(OBJECT_T_WINDOW, sizeof(*ret));
  ret->object.draw = window_draw;
  ret->object.set_dimensions = window_set_dimensions_cb;
  return ret;
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
  static struct window *test_win = NULL;
  if (!test_win) {
    test_win = window_new(50, 50);
    object_set_dimensions(test_win, 100, 100);
    object_set_position(test_win, 10, 10);
  }
  draw_color(0,0,0,0);
  clear_screen();
  object_draw(test_win, 10, 10);
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
