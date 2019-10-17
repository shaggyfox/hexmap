#include "engine.h"

enum object_type_e {OBJECT_T_WINDOW, OBJECT_T_WIDGET, OBJECT_T_LAYOUT};

struct win_object {
  enum object_type_e type;
  void (*get_dimensions)(void *data, int *w, int *h);
  void (*set_dimensions)(void *data, int w, int h);
  void (*set_position)(void *data, int x, int y);
  void (*get_position)(void *data, int *x, int *y);
  void (*draw)(void *data, int x, int y);
  void (*mouse)(void *data, int x, int y, SDL_Rect *mouse_position);
  void (*on_change)(void *data, void *cb_data);
  void *on_change_cb_data;
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

void object_set_on_change(void *object, void (*cb)(void*, void*), void* cb_data)
{
  struct win_object *ctx = object;
  ctx->on_change = cb;
  ctx->on_change_cb_data = cb_data;
}

void object_changed(void *object)
{
  struct win_object *ctx = object;
  if (ctx->on_change) {
    ctx->on_change(object, ctx->on_change_cb_data);
  }
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

void object_mouse(void *object, int x, int y, SDL_Rect *mouse_position)
{
  struct win_object *ctx = object;
  if (ctx->mouse) {
    ctx->mouse(object, x, y, mouse_position);
  } else {
    printf("DEBUG no mouse event handler\n");
  }
}


struct widget {
  struct win_object object;
};




/* ====== WIDGET: label ========== */
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
#include <stdarg.h>
void label_set(struct widget_label *ctx, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  if (ctx->text) {
    free(ctx->text);
  }
  vasprintf(&ctx->text, fmt, ap);
  va_end(ap);
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

/* ======== WIDGET: slider ======== */
struct widget_slider {
  struct widget widget;
  int range;
  int offset;
  int value;
};

void slider_get_dimensions(void *object, int *w, int *h)
{
  struct widget_slider *ctx = object;
  int pointer_width = 10;
  int pointer_height = 10;
  /* get minimum slider width */
  int minimum_width = ctx->range + pointer_width;
  /* if minimum slider width > minimum object_size return 'minimum_slider_size'
   * else return minimum object_size */
  if (minimum_width > ctx->widget.object.rect.w) {
    *w = minimum_width;
  } else {
    *w = ctx->widget.object.rect.w;
  }
  /* always return minimum slider height */
  *h = pointer_height;
}

void slider_draw(void *object, int x, int y)
{
  struct widget_slider *ctx = object;
  int w, h;
  slider_get_dimensions(object, &w, &h);
  draw_color(0,255,0,255);
  draw_fill_rect4(x + 10 / 2, y, w - 10, h);
  draw_color(0,0,255,255);
  draw_fill_rect4(x + (ctx->value * (w - 10)) / ctx->range, y + 5, 10, 5);
}

void slider_mouse(void *object, int x, int y, SDL_Rect *mouse_position)
{
  struct widget_slider *ctx = object;
  int w, h;
  slider_get_dimensions(object, &w, &h);
  ctx->value = (mouse_position->x - (x + 10 / 2)) * ctx->range / (w - 10);
  if (ctx->value < ctx->offset) {
    ctx->value = ctx->offset;
  }
  if (ctx->value > ctx->range + ctx->offset) {
    ctx->value = ctx->range + ctx->offset;
  }
  /* notify about the change */
  object_changed(object);
}

struct widget_slider *slider_new(int range, int value)
{
  struct widget_slider *ret = object_new(OBJECT_T_WIDGET, sizeof(*ret));
  ret->widget.object.draw = slider_draw;
  ret->widget.object.get_dimensions = slider_get_dimensions;
  ret->widget.object.mouse = slider_mouse;
  ret->range = range;
  ret->value = value;
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

static void box_foreach(void *object, int x, int y, enum layout_type_e layout_type,
    void (*cb)(void *ctx, void *object, SDL_Rect *rect, void *cb_data), void *cb_data)
{
  struct layout *ctx = object;
  int tmp_w, tmp_h;
  object_get_dimensions(object, &tmp_w, &tmp_h);
  for (int i = 0; i < ctx->count; ++i) {
    object_get_dimensions(ctx->entries[i].object, &tmp_w, &tmp_h);
    int draw_x = x + (ctx->entries[i].width - tmp_w) / 2;
    int draw_y = y + (ctx->entries[i].height - tmp_h) / 2;
    SDL_Rect rect = {
      .x = draw_x,
      .y = draw_y,
      .w = tmp_w,
      .h = tmp_h };
    cb(object, ctx->entries[i].object, &rect, cb_data);
    switch (layout_type) {
      case LAYOUT_T_VBOX:
        y += ctx->entries[i].height;
        break;
      case LAYOUT_T_HBOX:
        x += ctx->entries[i].width;
        break;
      default:
        break;
    }
  }

}

static void box_mouse_cb(void *ctx, void *object, SDL_Rect *rect, void *cb_data)
{
  SDL_Rect *mouse_position = cb_data;
  if (abs(rect->x + rect->w / 2 - mouse_position->x) < rect->w / 2 &&
      abs(rect->y + rect->h / 2 - mouse_position->y) < rect->h / 2) {
    object_mouse(object, rect->x, rect->y, cb_data);
  }
}

static void box_mouse(void *object, int x, int y, enum layout_type_e layout_type, SDL_Rect *mouse_position)
{
  box_foreach(object, x, y, layout_type, box_mouse_cb, mouse_position);
}

static void vbox_mouse(void *object, int x, int y, SDL_Rect *mouse_position)
{
  box_mouse(object, x, y , LAYOUT_T_VBOX, mouse_position);
}

static void box_draw_callback (void *object, void *entry, SDL_Rect *rect, void* cb_data)
{
  object_draw(entry, rect->x, rect->y);
}
static void box_draw(void *object, int x, int y, enum layout_type_e layout_type)
{
  draw_color(255,0,0,255);
  int tmp_w, tmp_h;
  object_get_dimensions(object, &tmp_w, &tmp_h);
  draw_rect4(x, y, tmp_w, tmp_h);
  box_foreach(object, x, y, layout_type, box_draw_callback, NULL);
}

static void hbox_draw(void *object, int x, int y)
{
  box_draw(object, x, y, LAYOUT_T_HBOX);
}

static void vbox_draw(void *object, int x, int y)
{
  box_draw(object, x, y, LAYOUT_T_VBOX);
}

static void box_set_dimensions(void *object, int w, int h, enum layout_type_e layout_type)
{
  struct layout *ctx = object;
  /* call parent */
  object_set_dimensions_cb(object, w, h);
  /* calculate the average size */
  /* to do this */
  int tmp_w, tmp_h;
  int height_left = h; /* XXX */
  int width_left = w; /* XXX */

  int expand_cnt = 0;
  for (int i = 0; i < ctx->count; ++i) {
    /* reset done flag */
    ctx->entries[i].flags &= ~DONE;
    switch (layout_type) {
      case LAYOUT_T_VBOX:
        object_set_dimensions(ctx->entries[i].object, w, 0); /* forces minimum height in get_dimensions */
        break;
      case LAYOUT_T_HBOX:
        object_set_dimensions(ctx->entries[i].object, 0, h); /* forces minimun width in get_dimensions */
        break;
      default:
        break;
    }
    // 1 find all objects missing the EXPAND flag
    if (!(ctx->entries[i].flags & EXPAND)) {
      // 2 get the minimum size from all shrinked objects
      object_get_dimensions(ctx->entries[i].object, &tmp_w, &tmp_h);
      // 3 decrease <width> by the sum of the size of all shrinked objects
      switch (layout_type) {
        case LAYOUT_T_VBOX:
          height_left -= tmp_h;
          ctx->entries[i].width = w;
          ctx->entries[i].height = tmp_h;
          break;
        case LAYOUT_T_HBOX:
          width_left -= tmp_w;
          ctx->entries[i].width = tmp_w;
          ctx->entries[i].height = h;
          break;
        default:
          break;
      }
      // ... and mark object as 'done'
      ctx->entries[i].flags |= DONE;
    } else {
      expand_cnt += 1;
    }
  }
  int repeat;
  int common_size;
  do {
    repeat = 0;
    // 4 divide <height> by the number of expanded-flagged objects
    // to get the common size
    switch (layout_type) {
      case LAYOUT_T_VBOX:
        common_size = height_left / expand_cnt;
        break;
      case LAYOUT_T_HBOX:
        common_size = width_left / expand_cnt;
        break;
      default:
        break;
    }
    for (int i = 0; i < ctx->count; ++i) {
      // 5 get minimum <height> for each expanded-flagged object
      if (ctx->entries[i].flags & EXPAND && !(ctx->entries[i].flags & DONE)) {
        // if the minimum size exceeds the average size
        object_get_dimensions(ctx->entries[i].object, &tmp_w, &tmp_h);
        if (
            (layout_type == LAYOUT_T_VBOX && common_size < tmp_h) ||
            (layout_type == LAYOUT_T_HBOX && common_size < tmp_w)) {
          // decrease it's minimum size from <width>, mark object as 'done' then
          // set new_average to  <with> divided by the count of remaining
          // expanded-flagged objects
          expand_cnt -= 1;
          repeat = 1;
          ctx->entries[i].flags |= DONE;

          switch (layout_type) {
            case LAYOUT_T_VBOX:
              height_left -= tmp_h;
              ctx->entries[i].width = w;
              ctx->entries[i].height = tmp_h;
              break;
            case LAYOUT_T_HBOX:
              width_left -= tmp_w;
              ctx->entries[i].width = tmp_w;
              ctx->entries[i].height = h;
              break;
            default:
              break;
          }
          break;
        }
      }
    }
  } while (repeat && expand_cnt);

  // 6 set all remaining (not 'done') objects to the average-size
  for (int i = 0; i < ctx->count; ++i) {
    if (!(ctx->entries[i].flags & DONE)) {
      switch (layout_type) {
        case LAYOUT_T_VBOX:
          object_set_dimensions(ctx->entries[i].object, w, common_size);
          ctx->entries[i].width = w;
          ctx->entries[i].height = common_size;
          break;
        case LAYOUT_T_HBOX:
          object_set_dimensions(ctx->entries[i].object, common_size, h);
          ctx->entries[i].width = common_size;
          ctx->entries[i].height = h;
          break;
        default:
          break;
      }
    }
  }
}

void vbox_set_dimensions(void *object, int w, int h)
{
  box_set_dimensions(object, w, h, LAYOUT_T_VBOX);
}

void hbox_set_dimensions(void *object, int w, int h)
{
  box_set_dimensions(object, w, h, LAYOUT_T_HBOX);
}

struct layout *vbox_new(void)
{
  struct layout *ret = object_new(OBJECT_T_LAYOUT, sizeof(*ret));
  ret->object.set_dimensions = vbox_set_dimensions;
  ret->object.mouse = vbox_mouse;
  ret->object.draw = vbox_draw;
  ret->add = layout_add_cb;
  return ret;
};

struct layout *hbox_new(void)
{
  struct layout *ret = object_new(OBJECT_T_LAYOUT, sizeof(*ret));
  ret->object.set_dimensions = hbox_set_dimensions;
  ret->object.draw = hbox_draw;
  ret->add = layout_add_cb;
  return ret;
};

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
    object_draw(win->layout, win->object.rect.x, win->object.rect.y);
  }
}

static void window_mouse_cb(void *object, int x, int y, SDL_Rect *mouse_position)
{
  struct window *ctx = object;
  if (ctx->layout) {
    /* ignore given x/y on window */
    x = ctx->object.rect.x;
    y = ctx->object.rect.y;
    int w = ctx->object.rect.w;
    int h = ctx->object.rect.h;
    if (
        (abs(x + w / 2 - mouse_position->x) < w / 2) &&
        (abs(y + h / 2 - mouse_position->y) < h / 2)
       ) {
      object_mouse(ctx->layout, x, y, mouse_position);
    }
  }
}

struct window *window_new(int w, int h)
{
  struct window *ret = object_new(OBJECT_T_WINDOW, sizeof(*ret));
  ret->object.draw = window_draw;
  ret->object.set_dimensions = window_set_dimensions_cb;
  ret->object.mouse = window_mouse_cb;
  return ret;
}

#include "list.h"

struct windowmanager {
  dlist window_list;
};

void windowmanager_draw(struct windowmanager *win_manager)
{
  for (dlist_iter *i = dlist_begin(&win_manager->window_list);
        i; i = dlist_next(i)) {
    object_draw(dlist_data(i), 0, 0);
  }
}

void windowmanager_mouse_move(struct windowmanager *win_manager, int x, int y)
{
}

void windowmanager_mouse_down(struct windowmanager *win_manager, int button, int x, int y)
{
  for (dlist_iter *i = dlist_begin(&win_manager->window_list); i; i = dlist_next(i)) {
    struct win_object *win = dlist_data(i);
    if (abs(win->rect.x + win->rect.w / 2 - x) < win->rect.w / 2 &&
        abs(win->rect.y + win->rect.h / 2 - y) < win->rect.h / 2) {
      SDL_Rect mouse_position = {.x = x, .y= y, .w = button};
      object_mouse(win, 0, 0, &mouse_position);
    }
  }
}

void windowmanager_mouse_up(struct windowmanager *win_manager, int button, int x, int y)
{
}

void windowmanager_add(struct windowmanager *win_manager, struct window *win)
{
  dlist_append(&win_manager->window_list, win);
}

static struct windowmanager glob_win_mgmt;

void mouse_up(int button, int x, int y, void *data)
{
}
void mouse_down(int button, int x, int y, void *data) {

  windowmanager_mouse_down(&glob_win_mgmt, button, x, y);
}

void mouse_motion(int x, int y, void *data)
{
}

static void init(void **data)
{
  *data = NULL;
}

static void update(void *data, float delta)
{
}

static void on_change1_cb(void *object, void *data)
{
  struct widget_slider *ctx = object;
  struct widget_label *ll = data;

  label_set(ll, "value: %d", ctx->value);
}

static void draw(void *data)
{
  static void *label;
  static void *label2;
  static struct layout *layout;
  static void *slider;
  static struct window *test_win = NULL;
  if (!test_win) {
    test_win = window_new(50, 50);
    layout = vbox_new();
    window_set_layout(test_win, layout);
   label = label_new("blabla");
   label2 = label_new("blabla2");
    layout_add(layout, label, "EXPAND");
    slider = slider_new(10,0);
    layout_add(layout, slider, "");
    layout_add(layout, label2, "");
    object_set_on_change(slider, on_change1_cb, label2);
    object_set_dimensions(test_win, 100, 100);
    object_set_position(test_win, 10, 10);
    windowmanager_add(&glob_win_mgmt, test_win);
  }
  draw_color(0,0,0,0);
  clear_screen();
  windowmanager_draw(&glob_win_mgmt);
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
  .game_on_mouse_down = mouse_down, //)(int x, int y, int button, void *data);
  .game_on_mouse_up = mouse_up, //)(int x, int y, int button, void *data);
  .game_on_mouse_motion = mouse_motion, //)(int x, int y, void *data);
};

ENGINE_MAIN(&ctx);
