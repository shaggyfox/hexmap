#include "engine.h"
#include <assert.h>

enum object_type_e {OBJECT_T_WINDOW, OBJECT_T_WIDGET, OBJECT_T_LAYOUT};
enum event_type_e {EVENT_T_MOUSE};
struct event_st {
  enum event_type_e type;
};

struct mouse_event {
  struct event_st event;
  enum {MOUSE_BUTTON_DOWN, MOUSE_BUTTON_UP, MOUSE_MOVE} type;
  int x;
  int y;
  int buttons;
};

typedef struct event_st event;

struct win_object {
  enum object_type_e type;
  void (*get_dimensions)(void *data, int *w, int *h);
  void (*set_dimensions)(void *data, int w, int h);
  void (*set_position)(void *data, int x, int y);
  void (*get_position)(void *data, int *x, int *y);
  void (*draw)(void *data);
  struct win_object *(*event)(void *data, struct event_st *event, void *cb_data);
  void *event_cb_data;
  void (*on_change)(void *data, void *cb_data);
  void *on_change_cb_data;
  SDL_Rect rect;
};

static void object_set_dimensions_default_handler(void *object, int w, int h)
{
  struct win_object *ctx = object;
  ctx->rect.w = w;
  ctx->rect.h = h;
}

static void object_set_position_default_handler(void *object, int x, int y)
{
  struct win_object *ctx = object;
  ctx->rect.x = x;
  ctx->rect.y = y;
}

static void object_get_position_default_handler(void *object, int *x, int *y)
{
  struct win_object *ctx = object;
  *x = ctx->rect.x;
  *y = ctx->rect.y;
}

static void object_get_dimensions_default_handler(void *object, int *w, int *h)
{
  struct win_object *ctx = object;
  *w = ctx->rect.w;
  *h = ctx->rect.h;
}

void *object_new(enum object_type_e type, int size)
{
  struct win_object *ret = calloc(1, size);
  ret->type = type;
  ret->set_dimensions = object_set_dimensions_default_handler;
  ret->get_dimensions = object_get_dimensions_default_handler;
  ret->set_position = object_set_position_default_handler;
  ret->get_position = object_get_position_default_handler;
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

void object_set_position(void *object, int x, int y)
{
  struct win_object *ctx = object;
  ctx->set_position(object, x, y);
}

void object_get_position(void *object, int *x, int *y)
{
  struct win_object *ctx = object;
  ctx->get_position(object, x, y);
}

void object_draw(void *object)
{
  struct win_object *ctx = object;
  /* update position */
  int tmp_x, tmp_y;
  object_get_position(object, &tmp_x, &tmp_y);
  object_set_position(object, tmp_x, tmp_y);
  ctx->draw(object);
}

void *object_inject_event(void *object, struct event_st *event)
{
  struct win_object *ctx = object;
  if (ctx->event) {
    return ctx->event(object, event, ctx->event_cb_data);
  } else {
    printf("DEBUG no event handler\n");
  }
  return NULL;
}

void *object_inject_mouse_event_down(void *object, int button, int x, int y)
{
  struct mouse_event event;
  event.event.type = EVENT_T_MOUSE;
  event.buttons = button;
  event.x = x;
  event.y = y;
  event.type = MOUSE_BUTTON_DOWN;
  return object_inject_event(object, (void*)&event);
}

void *object_inject_mouse_event_up(void *object, int button, int x, int y)
{
  struct mouse_event event;
  event.event.type = EVENT_T_MOUSE;
  event.buttons = button;
  event.x = x;
  event.y = y;
  event.type = MOUSE_BUTTON_UP;
  return object_inject_event(object, (void*)&event);
}

void *object_inject_mouse_event_move(void *object, int x, int y)
{
  struct mouse_event event;
  event.event.type = EVENT_T_MOUSE;
  event.buttons = 0;
  event.x = x;
  event.y = y;
  event.type = MOUSE_MOVE;
  return object_inject_event(object, (void*)&event);
}

struct widget {
  struct win_object object;
};

/* ===================================================================== */
/* ======================== WIDGET: space ============================== */
/* ===================================================================== */

struct widget_space {
  struct widget widget;
  int min_width;
  int min_height;
};

static void space_get_dimensions_handler(void *object, int *w, int *h)
{
  struct widget_space *ctx = object;
  if (ctx->min_width > ctx->widget.object.rect.w) {
    *w = ctx->min_width;
  } else {
    *w = ctx->widget.object.rect.w;
  }

  if (ctx->min_height > ctx->widget.object.rect.h) {
    *h = ctx->min_height;
  } else {
    *h = ctx->widget.object.rect.h;
  }
}

void space_draw(void *object)
{
  /* do nothing */
#ifdef DEBUG
  struct win_object *ctx = object;
  int tmp_w, tmp_h;
  object_get_dimensions(object, &tmp_w, &tmp_h);
  draw_color(0,0,255,255);
  draw_rect4(ctx->rect.x, ctx->rect.y, tmp_w, tmp_h);
#endif
}

struct widget_space *space_new(int w, int h)
{
  struct widget_space *ret = object_new(OBJECT_T_WIDGET, sizeof(*ret));
  ret->min_width = w;
  ret->min_height = h;
  ret->widget.object.get_dimensions = space_get_dimensions_handler;
  ret->widget.object.draw = space_draw;
  return ret;
}

/* ===================================================================== */
/* ======================== WIDGET: label ============================== */
/* ===================================================================== */
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

void label_draw(void *object)
{
  struct widget_label *ctx = object;
  draw_text(ctx->widget.object.rect.x, ctx->widget.object.rect.y, ctx->text);
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


/* ===================================================================== */
/* ======================== WIDGET: Checkbox =========================== */
/* ===================================================================== */

struct widget_container {
  struct widget widget;
  void *object;
};


struct layout *vbox_new(void);
struct layout *hbox_new(void);
void layout_add(struct layout *layout, void *object, const char *flags);

void container_get_dimensions_handler(void *object, int *w, int *h)
{
  struct widget_container *ctx = object;
  object_get_dimensions(ctx->object, w, h);
}

void container_set_dimensions_handler(void *object, int w, int h)
{
  struct widget_container *ctx = object;
  object_set_dimensions(ctx->object, w, h);
}

void container_get_position_handler(void *object, int *x, int *y)
{
  struct widget_container *ctx = object;
  object_get_position(ctx->object, x, y);
}

void container_set_position_handler(void *object, int x, int y)
{
  struct widget_container *ctx = object;
  object_set_position(ctx->object, x, y);
}

void container_draw_handler(void *object)
{
  struct widget_container *ctx = object;
  object_draw(ctx->object);
}

struct win_object *container_event_handler(void *object, struct event_st *event, void *cb_data)
{
  struct widget_container *ctx = object;
  return object_inject_event(ctx->object, event);
}

void container_set_object(struct widget_container *ctx, void *object)
{
  ctx->object = object;
}

struct widget_container *container_new(void *object)
{
  struct widget_container *ret = object_new(OBJECT_T_WIDGET, sizeof(*ret));
  ret->widget.object.draw = container_draw_handler;
  ret->widget.object.get_position = container_get_position_handler;
  ret->widget.object.set_position = container_set_position_handler;
  ret->widget.object.get_dimensions = container_get_dimensions_handler;
  ret->widget.object.set_dimensions = container_set_dimensions_handler;
  ret->widget.object.event = container_event_handler;
  ret->object = object;
  return ret;
}

/* ===================================================================== */
/* ======================== WIDGET: Radiobuttons ======================= */
/* ===================================================================== */

struct widget_checkbox {
  struct widget_container container;
  int v;
  /* ... */
};

struct win_object *widget_checkbox_event_handler(void *object, struct event_st *event, void *cb_data)
{
  struct widget_checkbox *cb = cb_data;
  struct widget_label *ctx = object;
  cb->v = !cb->v;
  label_set(ctx, "%s", cb->v ? "1": "0");
  return NULL;
}

struct widget_checkbox *checkbox_new(char *name)
{
  struct layout *layout = hbox_new();
  struct widget_checkbox *ret = (void*)container_new(layout);
  ret = realloc(ret, sizeof(*ret));
  struct widget_label *labela = label_new("0");
  struct widget_label *labelb = label_new(name);
  labela->widget.object.event = widget_checkbox_event_handler;
  labela->widget.object.event_cb_data = ret;
  layout_add(layout, labela, "");
  layout_add(layout, labelb, "LEFT");
  ret->v = 1;
  return ret;
}


/* ===================================================================== */
/* ======================== WIDGET: Radiobuttons ======================= */
/* ===================================================================== */


/* ===================================================================== */
/* ======================== WIDGET: button ============================== */
/* ===================================================================== */
struct widget_button {
  struct widget widget;
  char *text;
  int font;
  void (*on_click_cb)(void *obj, void *data);
  void *on_click_cb_data;
};

void button_get_dimensions(void *object, int *w, int *h)
{
  struct widget_button *ctx = object;
  SDL_Rect rect;
  text_dimensions(ctx->text, &rect);
  *w = rect.w + 4;
  *h = rect.h + 4;
}

void button_draw(void *object)
{
  struct widget_button *ctx = object;
  int x, y, w, h;
  object_get_position(object, &x, &y);
  object_get_dimensions(object, &w, &h);
  draw_color(180,180,180,255);
  draw_fill_rect4(x, y, w, h);
  draw_color(100,100,100,255);
  draw_rect4(x, y, w, h);
  draw_color(0,0,0,255);
  draw_text(x + 2, y + 2, ctx->text);
}

struct win_object *button_event(void *object, struct event_st *event, void *cb_data)
{
  if (event->type == EVENT_T_MOUSE) {
    struct mouse_event *mouse = (void*)event;
    int x, y, w, h;
    object_get_position(object, &x, &y);
    object_get_dimensions(object, &w, &h);
    if (abs(x + w / 2 - mouse->x) < w / 2 &&
        abs(y + h / 2 - mouse->y) < h / 2) {
      if (mouse->type == MOUSE_BUTTON_DOWN) {
        return object;
      } else if (mouse->type == MOUSE_BUTTON_UP) {
        struct widget_button *ctx = object;
        printf("button pressed\n");
        if (ctx->on_click_cb) {
          ctx->on_click_cb(object, ctx->on_click_cb_data);
        }
      }
    }
  }
  return NULL;
}

struct widget_button *button_new(char *text)
{
  struct widget_button *ret = object_new(OBJECT_T_WIDGET, sizeof(*ret));
  ret->widget.object.draw = button_draw;
  ret->widget.object.get_dimensions = button_get_dimensions;
  ret->widget.object.event = button_event;
  ret->text = strdup(text);
  ret->font = FONT_DEFAULT;
  return ret;
}

/* ===================================================================== */
/* ======================== WIDGET: slider ============================= */
/* ===================================================================== */

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

void slider_draw(void *object)
{
  struct widget_slider *ctx = object;
  int w, h;
  int x = ctx->widget.object.rect.x;
  int y = ctx->widget.object.rect.y;
  slider_get_dimensions(object, &w, &h);
  draw_color(0,255,0,255);
  draw_fill_rect4(x + 10 / 2, y, w - 10, h);
  draw_color(0,0,255,255);
  draw_fill_rect4(x + (ctx->value * (w - 10)) / ctx->range, y + 5, 10, 5);
}

struct win_object *slider_mouse(void *object, struct mouse_event *mouse)
{
  struct widget_slider *ctx = object;
  int w, h;
  int x = ctx->widget.object.rect.x;
  //int y = ctx->widget.object.rect.y;
  slider_get_dimensions(object, &w, &h);
  ctx->value = (mouse->x - (x + 10 / 2)) * ctx->range / (w - 10);
  if (ctx->value < ctx->offset) {
    ctx->value = ctx->offset;
  }
  if (ctx->value > ctx->range + ctx->offset) {
    ctx->value = ctx->range + ctx->offset;
  }
  /* notify about the change */
  object_changed(object);
  return object;
}

struct win_object *slider_event(void *object, struct event_st *event, void *cb_data)
{
  switch (event->type) {
    case EVENT_T_MOUSE:
      return slider_mouse(object, (struct mouse_event *)event);
    default:
      break;
  }
  return NULL;
}

struct widget_slider *slider_new(int range, int value)
{
  struct widget_slider *ret = object_new(OBJECT_T_WIDGET, sizeof(*ret));
  ret->widget.object.draw = slider_draw;
  ret->widget.object.get_dimensions = slider_get_dimensions;
  ret->widget.object.event = slider_event;
  ret->range = range;
  ret->value = value;
  return ret;
}


/* ===================================================================== */
/* ======================== LAYOUT: GENERIC  =========================== */
/* ===================================================================== */

/* layout flags */
#define DONE 1
#define EXPAND 2
#define HIDDEN 4
#define LEFT 8
#define RIGHT 16
#define TOP 32
#define BOTTOM 64
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
  enum layout_type_e type;
  struct layout_widget_entry *entries; /* child objects */
  int count; /* layout_widget_entry count */
  void (*add)(struct layout*, void *object, const char *flags);
};

static void layout_add_handler(struct layout *layout, void *object, const char *flags)
{
  struct layout_widget_entry *entry;
  layout->entries = realloc(layout->entries, sizeof(*layout->entries) * (layout->count + 1));
  entry = &layout->entries[layout->count++];
  entry->object = object;
  entry->flags = 0;
  if (strcasestr(flags, "expand")) {
    entry->flags |= EXPAND;
  }
  if (strcasestr(flags, "left")) {
    entry->flags |= LEFT;
  }
  if (strcasestr(flags , "right")) {
    entry->flags |= RIGHT;
  }
  if (strcasestr(flags, "top")) {
    entry->flags |= TOP;
  }
  if (strcasestr(flags, "bottom")) {
    entry->flags |= BOTTOM;
  }
}

void layout_add(struct layout *layout, void *object, const char *flags)
{
  layout->add(layout, object, flags);
}

static struct win_object *box_foreach(void *object, int x, int y,
    void* (*cb)(void *ctx, struct layout_widget_entry *object, SDL_Rect *rect, void *cb_data), void *cb_data)
{
  void *ret = NULL;
  struct layout *ctx = object;
  int tmp_w, tmp_h;
  object_get_dimensions_default_handler(object, &tmp_w, &tmp_h);
  for (int i = 0; i < ctx->count; ++i) {
    object_get_dimensions(ctx->entries[i].object, &tmp_w, &tmp_h);
    int draw_x = x;
    int draw_y = y;
    if (ctx->entries[i].flags & LEFT) {
      /* left align */
    } else if (ctx->entries[i].flags & RIGHT) {
      /* right align */
      draw_x += ctx->entries[i].width - tmp_w;
    } else {
      /* center x */
      draw_x += (ctx->entries[i].width - tmp_w) / 2;
    }
    /* center y */
    if (ctx->entries[i].flags & TOP) {
    } else if (ctx->entries[i].flags & BOTTOM) {
      draw_y += ctx->entries[i].height - tmp_h;
    } else {
      draw_y += (ctx->entries[i].height - tmp_h) / 2;
    }
    SDL_Rect rect = {
      .x = draw_x,
      .y = draw_y,
      .w = tmp_w,
      .h = tmp_h };
    if ((ret = cb(object, &ctx->entries[i], &rect, cb_data))) {
      break;
    }
    switch (ctx->type) {
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
  return ret;
}

static void *box_mouse_event_foreach_cb(void *ctx, struct layout_widget_entry *entry, SDL_Rect *rect, void *cb_data)
{
  struct mouse_event *mouse = cb_data;
  if (abs(rect->x + rect->w / 2 - mouse->x) < rect->w / 2 &&
      abs(rect->y + rect->h / 2 - mouse->y) < rect->h / 2) {
    /* stop loop */
    return object_inject_event(entry->object, cb_data);
  }
  return NULL; /* keep going */
}

static struct win_object *box_event_handler(void *object, struct event_st *event, void *cb_data)
{
  struct win_object *ctx = object;
  if (event->type == EVENT_T_MOUSE) {
    return box_foreach(object, ctx->rect.x, ctx->rect.y, box_mouse_event_foreach_cb, event);
  }
  return NULL;
}

static void *box_draw_foreach_callback (void *object, struct layout_widget_entry *entry, SDL_Rect *rect, void* cb_data)
{
  object_draw(entry->object);
  return NULL; /* keep going */
}

static void box_draw_handler(void *object)
{
  draw_color(255,0,0,255);
  int tmp_w, tmp_h;
  int x, y;
  object_get_position(object, &x, &y);
  object_get_dimensions(object, &tmp_w, &tmp_h);
  draw_rect4(x, y, tmp_w, tmp_h);
  box_foreach(object, x, y, box_draw_foreach_callback, NULL);
}

static void *box_set_position_cb(void *object, struct layout_widget_entry *entry, SDL_Rect *rect, void *data)
{
  object_set_position(entry->object, rect->x, rect->y);
  return NULL; /* keep on going */
}

static void box_set_position(void *object, int x, int y)
{
  /* call parent */
  object_set_position_default_handler(object, x, y);
  box_foreach(object, x, y, box_set_position_cb, NULL);
}

static void* vbox_size_cb(void *object, struct layout_widget_entry *entry, SDL_Rect *rect, void *cb_data)
{
  SDL_Rect *out_rect = cb_data;
  int w = 0;
  if (rect->w > entry->width) {
    w = rect->w;
  } else {
    w = entry->width;
  }
  int h = 0;
  if (rect->h > entry->height) {
    h = rect->h;
  } else {
    h = entry->height;
  }
  if (w > out_rect->w) {
    out_rect->w = w;
  }
  out_rect->h += h;
  return NULL;
}

static void* hbox_size_cb(void *object, struct layout_widget_entry *entry, SDL_Rect *rect, void *cb_data)
{
  SDL_Rect *out_rect = cb_data;
  int h = 0;
  if (rect->h > entry->height) {
    h = rect->h;
  } else {
    h = entry->height;
  }
  int w = 0;
  if (rect->w > entry->width) {
    w = rect->w;
  } else {
    w = entry->width;
  }
  if (h > out_rect->h) {
    out_rect->h = h;
  }
  out_rect->w += w;
  return NULL;
}

static void box_get_dimensions(void *object, int *w, int *h)
{
  struct layout *ctx = object;
  SDL_Rect rect = {0};

  switch(ctx->type) {
    case LAYOUT_T_HBOX:
      box_foreach(object, 0, 0, hbox_size_cb, &rect);
      break;
    case LAYOUT_T_VBOX:
      box_foreach(object, 0, 0, vbox_size_cb, &rect);
      break;
    default:
      break;
  }
  *w = rect.w;
  *h = rect.h;
/*  if (*w < ctx->object.rect.w) {
    *w = ctx->object.rect.w;
  }
  if (*h < ctx->object.rect.h) {
    *h = ctx->object.rect.h;
  }*/
}

static void box_set_dimensions(void *object, int w, int h, enum layout_type_e layout_type)
{
  struct layout *ctx = object;
  /* call parent */
  object_set_dimensions_default_handler(object, w, h);
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
          ctx->entries[i].width = w > tmp_w ? w : tmp_w;
          ctx->entries[i].height = tmp_h;
          break;
        case LAYOUT_T_HBOX:
          width_left -= tmp_w;
          ctx->entries[i].width = tmp_w;
          ctx->entries[i].height = h > tmp_h ? h : tmp_h;
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
  int common_size;
  if (expand_cnt) {
    int repeat;
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
          assert(0);
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
                ctx->entries[i].width = w > tmp_w ? w : tmp_w;
                ctx->entries[i].height = tmp_h;
                break;
              case LAYOUT_T_HBOX:
                width_left -= tmp_w;
                ctx->entries[i].width = tmp_w;
                ctx->entries[i].height = h > tmp_h ? h : tmp_h;
                break;
              default:
                assert(0);
                break;
            }
            break;
          }
        }
      }
    } while (repeat && expand_cnt);
  } else {
    switch (layout_type) {
      case LAYOUT_T_VBOX:
        common_size = height_left;
        break;
      case LAYOUT_T_HBOX:
        common_size = width_left;
        break;
      default:
        break;
    }
  }

  // 6 set all remaining (not 'done') objects to the average-size
  for (int i = 0; i < ctx->count; ++i) {
    if (!(ctx->entries[i].flags & DONE)) {
      switch (layout_type) {
        case LAYOUT_T_VBOX:
          object_set_dimensions(ctx->entries[i].object, w, common_size);
          ctx->entries[i].width = w > tmp_w ? w : tmp_w;
          ctx->entries[i].height = common_size;
          break;
        case LAYOUT_T_HBOX:
          object_set_dimensions(ctx->entries[i].object, common_size, h);
          ctx->entries[i].width = common_size;
          ctx->entries[i].height = h > tmp_h ? h : tmp_h;
          break;
        default:
          break;
      }
    }
  }
}

/* ===================================================================== */
/* ============================= LAYOUT HBOX =========================== */
/* ===================================================================== */

void hbox_set_dimensions(void *object, int w, int h)
{
  box_set_dimensions(object, w, h, LAYOUT_T_HBOX);
}

struct layout *hbox_new(void)
{
  struct layout *ret = object_new(OBJECT_T_LAYOUT, sizeof(*ret));
  ret->object.set_dimensions = hbox_set_dimensions; /* XXX use generic box function */
  ret->object.get_dimensions = box_get_dimensions;
  ret->object.draw = box_draw_handler;
  ret->object.event = box_event_handler;
  ret->object.set_position = box_set_position;
  ret->type = LAYOUT_T_HBOX;
  ret->add = layout_add_handler;
  return ret;
};

/* ===================================================================== */
/* ============================= LAYOUT VBOX =========================== */
/* ===================================================================== */


void vbox_set_dimensions(void *object, int w, int h)
{
  box_set_dimensions(object, w, h, LAYOUT_T_VBOX);
}

struct layout *vbox_new(void)
{
  struct layout *ret = object_new(OBJECT_T_LAYOUT, sizeof(*ret));
  ret->object.set_dimensions = vbox_set_dimensions; /* XXX use generic box function */
  ret->object.get_dimensions = box_get_dimensions;
  ret->object.event = box_event_handler;
  ret->object.draw = box_draw_handler;
  ret->object.set_position = box_set_position;
  ret->add = layout_add_handler;
  ret->type = LAYOUT_T_VBOX;
  return ret;
};


/* ===================================================================== */
/* =========================== WINDOW OBJECT =========================== */
/* ===================================================================== */

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
  object_set_dimensions_default_handler(object, w, h);
  if (ctx->layout) {
    object_set_dimensions(ctx->layout, w, h);
  }
}

void window_draw(void *data)
{
  struct window *win = data;
  draw_color(255,255,255,255);
  draw_rect(&win->object.rect);
  if (win->layout) {
    object_set_position(win->layout, win->object.rect.x, win->object.rect.y);
    object_draw(win->layout);
  }
}

static struct win_object *window_event_cb(void *object, struct event_st *event, void *cb_data)
{
  struct window *ctx = object;
  if (ctx->layout) {
    if (event->type == EVENT_T_MOUSE) {
      struct mouse_event *mouse = (void*) event;
      int x = ctx->object.rect.x;
      int y = ctx->object.rect.y;
      int w = ctx->object.rect.w;
      int h = ctx->object.rect.h;
      if (
          (abs(x + w / 2 - mouse->x) < w / 2) &&
          (abs(y + h / 2 - mouse->y) < h / 2)
         ) {
        return object_inject_event(ctx->layout, event);
      }
    }
  }
  return NULL;
}

struct window *window_new(int w, int h)
{
  struct window *ret = object_new(OBJECT_T_WINDOW, sizeof(*ret));
  ret->object.draw = window_draw;
  ret->object.set_dimensions = window_set_dimensions_cb;
  ret->object.event = window_event_cb;
  return ret;
}

/* ===================================================================== */
/* ========================== WINDOW MANAGER =========================== */
/* ===================================================================== */

#include "list.h"
struct windowmanager {
  dlist window_list;
  int button_down_value;
  void *current_event_object;
};

void windowmanager_draw(struct windowmanager *win_manager)
{
  for (dlist_iter *i = dlist_begin(&win_manager->window_list);
        i; i = dlist_next(i)) {
    object_draw(dlist_data(i));
  }
}

void windowmanager_mouse_move(struct windowmanager *win_manager, int x, int y)
{
  if (win_manager->current_event_object) {
    object_inject_mouse_event_move(win_manager->current_event_object, x, y);
  }
}

void windowmanager_mouse_down(struct windowmanager *win_manager, int button, int x, int y)
{
  if (win_manager->current_event_object) {
    object_inject_mouse_event_down(win_manager->current_event_object, button, x, y);
  } else {
    for (dlist_iter *i = dlist_begin(&win_manager->window_list); i; i = dlist_next(i)) {
      struct win_object *win = dlist_data(i);
      if (abs(win->rect.x + win->rect.w / 2 - x) < win->rect.w / 2 &&
          abs(win->rect.y + win->rect.h / 2 - y) < win->rect.h / 2) {
        void *ret = object_inject_mouse_event_down(win, button, x, y);
        if (!win_manager->current_event_object) {
          win_manager->current_event_object = ret;
          win_manager->button_down_value = button;
        }
      }
    }
  }
}

void windowmanager_mouse_up(struct windowmanager *win_manager, int button, int x, int y)
{
  if (win_manager->current_event_object) {
    object_inject_mouse_event_up(win_manager->current_event_object, button, x, y);
    win_manager->current_event_object = NULL;
  }
}

void windowmanager_add(struct windowmanager *win_manager, struct window *win)
{
  dlist_append(&win_manager->window_list, win);
}




/* ===================================================================== */
/* ========================== GAMELIB CALLBACKS ======================== */
/* ===================================================================== */



static struct windowmanager glob_win_mgmt;

void mouse_up(int button, int x, int y, void *data)
{
  windowmanager_mouse_up(&glob_win_mgmt, button, x, y);
}
void mouse_down(int button, int x, int y, void *data) {

  windowmanager_mouse_down(&glob_win_mgmt, button, x, y);
}

void mouse_motion(int x, int y, void *data)
{
  windowmanager_mouse_move(&glob_win_mgmt, x, y);
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
  static void *space;
  static struct layout *layout;
  static void *slider;
  static struct window *test_win = NULL;
  if (!test_win) {
    test_win = window_new(50, 50);
    layout = vbox_new();
    window_set_layout(test_win, layout);
    label = label_new("blabla");
    space = space_new(0, 10);
    label2 = label_new("blabla2");
    layout_add(layout, label, "EXPAND");
    slider = slider_new(10,0);
    layout_add(layout, slider, "");
    layout_add(layout, space, "");
    layout_add(layout, label2, "");
    void *button1 = button_new("button");
    layout_add(layout, button1, "");
    object_set_on_change(slider, on_change1_cb, label2);
    void *checkbox12 = checkbox_new("checkbox");
    layout_add(layout, checkbox12, "");

    object_set_dimensions(test_win, 100, 100);

    object_set_position(test_win, 50, 50);
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
