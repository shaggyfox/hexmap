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

void label_draw(void *object, int x, int y)
{
  struct widget_label *ctx = object;
  draw_text(x, y, ctx->text);
}

struct widget_label *label_new(char *text)
{
  struct widget_label *ret = object_new(OBJECT_T_WIDGET, sizeof(*ret));
  ret->widget.object.draw = label_draw;
  ret->widget.object.get_dimensions = label_get_dimensions;
  ret->text = strdup(text);
  ret->font = FONT_DEFAULT;
  return ret;
}

/* layout flags */
#define DONE 1
#define EXPAND 2
#define HIDDEN 4
struct layout_widget_entry
{
  int flags;
  void *object;
  int width;
  int height;
};

enum layout_type_e {LAYOUT_T_HBOX, LAYOUT_T_VBOX, LAYOUT_T_TABLE};
struct layout {
  struct win_object object;
  struct layout_widget_entry *entries; /* child objects */
  int count; /* layout_widget_entry count */
  void (*add)(struct layout*, void *object, const char *flags);
};

static void layout_add_cb(struct layout *layout, void *object, const char *flags)
{
  struct layout_widget_entry *entry;
  layout->entries = realloc(layout->entries, sizeof(*layout->entries) * (layout->count + 1));
  entry = &layout->entries[layout->count++];
  entry->object = object;
  entry->flags = 0;
  if (strcasestr(flags, "expand")) {
    entry->flags |= EXPAND;
  }
}

void layout_add(struct layout *layout, void *object, const char *flags)
{
  layout->add(layout, object, flags);
}
void vbox_draw(void *object, int x, int y)
{
  struct layout *ctx = object;
  draw_color(255,0,0,255);
  int tmp_w, tmp_h;
  object_get_dimensions(object, &tmp_w, &tmp_h);
  draw_rect4(x, y, tmp_w, tmp_h);
  for (int i = 0; i < ctx->count; ++i) {
    object_get_dimensions(ctx->entries[i].object, &tmp_w, &tmp_h);
    int draw_x = x + (ctx->entries[i].width - tmp_w) / 2;
    int draw_y = y + (ctx->entries[i].height - tmp_h) / 2;
    object_draw(ctx->entries[i].object, draw_x, draw_y);
    y += ctx->entries[i].height;
  }
}

void vbox_set_dimensions(void *object, int w, int h)
{
  struct layout *ctx = object;
  /* call parent */
  object_set_dimensions_cb(object, w, h);
  /* calculate the average size */
  /* to do this */
  int tmp_w, tmp_h;
  int height_left = h;
  int expand_cnt = 0;
  for (int i = 0; i < ctx->count; ++i) {
    /* reset done flag */
    ctx->entries[i].flags &= ~DONE;
    object_set_dimensions(ctx->entries[i].object, w, 0); /* forces minimum height in get_dimensions */
    // 1 find all objects missing the EXPAND flag
    if (!(ctx->entries[i].flags & EXPAND)) {
      // 2 get the minimum size from all shrinked objects
      object_get_dimensions(ctx->entries[i].object, &tmp_w, &tmp_h);
      // 3 decrease <width> by the sum of the size of all shrinked objects
      height_left -= tmp_h;
      // ... and mark object as 'done'
      ctx->entries[i].flags |= DONE;
      ctx->entries[i].width = w;
      ctx->entries[i].height = tmp_h;
    } else {
      expand_cnt += 1;
    }
  }
  int repeat;
  int common_height;
  do {
    repeat = 0;
    // 4 divide <height> by the number of expanded-flagged objects
    // to get the common size
    common_height = height_left / expand_cnt;
    for (int i = 0; i < ctx->count; ++i) {
      // 5 get minimum <height> for each expanded-flagged object
      if (ctx->entries[i].flags & EXPAND && !(ctx->entries[i].flags & DONE)) {
        // if the minimum size exceeds the average size
        object_get_dimensions(ctx->entries[i].object, &tmp_w, &tmp_h);
        if (common_height < tmp_h) {
          // decrease it's minimum size from <width>, mark object as 'done' then
          // set new_average to  <with> divided by the count of remaining
          // expanded-flagged objects
          height_left -= tmp_h;
          expand_cnt -= 1;
          repeat = 1;
          ctx->entries[i].flags |= DONE;
          ctx->entries[i].width = w;
          ctx->entries[i].height = tmp_h;
          break;
        }
      }
    }
  } while (repeat);

  // 6 set all remaining (not 'done') objects to the average-size
  for (int i = 0; i < ctx->count; ++i) {
    if (!(ctx->entries[i].flags & DONE)) {
      object_set_dimensions(ctx->entries[i].object, w, common_height);
      ctx->entries[i].width = w;
      ctx->entries[i].height = common_height;
    }
  }
}

void box_set_dimensions(void *object, enum layout_type_e type, int w, int h)
{
}

void hbox_set_dimensions(void *object, int w, int h)
{
}

struct layout *vbox_new(void)
{
  struct layout *ret = object_new(OBJECT_T_LAYOUT, sizeof(*ret));
  ret->object.set_dimensions = vbox_set_dimensions;
  ret->object.draw = vbox_draw;
  ret->add = layout_add_cb;
  return ret;
};

#if 0
struct layout *new_vbox(void)
{
  struct layout *ret = object_new(OBJECT_T_LAYOUT, sizeof(*ret));
  ret->object.set_dimensions = vbox_set_dimensions;
  ret->add = layout_add;
  return ret;
};
#endif

struct window {
  struct win_object object;
  struct layout *layout;
};

void window_set_layout(struct window *window, struct layout *layout)
{
  window->layout = layout;
}

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
  if (win->layout) {
    object_draw(win->layout, x, y);
  }
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
  static void *label;
  static void *label2;
  static struct layout *layout;
  if (!test_win) {
    test_win = window_new(50, 50);
    layout = vbox_new();
    window_set_layout(test_win, layout);
    label = label_new("blabla");
    label2 = label_new("blabla2");
    layout_add(layout, label, "EXPAND");
    layout_add(layout, label2, "");
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
  .screen_scale = 2,
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
