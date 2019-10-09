#include "engine.h"
#include "hex.h"
#include "hextile.h"

int mouse_is_down = 0;
struct map_pos center_pos = {0,0};
struct map_pos scroll_pos;
struct screen_pos mouse_pos = {0,0};
struct screen_pos current_mouse_pos = {0,0};
struct tileset *glob_tiles = NULL;

static void init(void **data)
{
  draw_color(0,0,0,255);
  text_color(150,150,150,255);
  glob_tiles = tileset_load_raw_from_file("../hextile.png", WIDTH, HEIGHT);
}

static void screen2map(struct screen_pos *s_pos, struct map_pos *m_pos)
{
  struct cube_pos c_pos;
  screen2cube(s_pos, &c_pos);
  cube2map(&c_pos, m_pos);
}

static void map2screen(struct map_pos *m_pos, struct screen_pos *s_pos)
{
  struct cube_pos c_pos;
  map2cube(m_pos, &c_pos);
  cube2screen(&c_pos, s_pos);
}

static void update(void *data, float delta)
{
}

static void motion(int x, int y, void *data)
{
  current_mouse_pos.x = x;
  current_mouse_pos.y = y;
  if (!mouse_is_down) {
    return;
  }
  struct screen_pos mouse_motion_pos = {x, y};
  mouse_motion_pos.x -= mouse_pos.x;
  mouse_motion_pos.y -= mouse_pos.y;
  screen2map(&mouse_motion_pos, &scroll_pos);
}

static void mouse_up(int x, int y, int button, void *data)
{
  mouse_is_down = 0;
  center_pos.x += scroll_pos.x;
  center_pos.y += scroll_pos.y;
  scroll_pos.x = 0;
  scroll_pos.y = 0;
}

static void mouse_down(int button, int x, int y, void *data)
{
  mouse_is_down = 1;
  mouse_pos.x = x;
  mouse_pos.y = y;
}

struct mmap {
  int w;
  int h;
  int *data;
};

static void mmap_init(struct mmap *map, int w, int h, int v)
{
  map->w = w;
  map->h = h;
  map->data = malloc(w * h * sizeof(*map->data));
  for (int i = 0; i < w * h; ++i) {
    map->data[i] = v;
  }
}

static void mmap_set(struct mmap* map, int x, int y, int v)
{
  while (y < 0) {
    y += map->h;
  }
  x -= y / 2;
  while (x < 0) {
    x += map->w;
  }
  map->data[(y%map->h) * map->w + (x%map->w)] = v;
}

static int mmap_get(struct mmap* map, int x, int y)
{
  return map->data[map->w * y + (x % map->w)];
}

static void map_normalize_coordinates(struct mmap* map, int *x_par, int *y_par)
{
  int x = *x_par;
  int y = *y_par;
  while (y < 0) {
    y += map->h;
    x -= map->h / 2;
  }
  while (y >= map->h) {
    y -= map->h;
    x += map->h/ 2;
  }
  while (x < 0) {
    x += map->w;
  }
  x %= map->w;
  *x_par = x;
  *y_par = y;
}

static void draw_map_test(int x, int y, int w, int h, struct map_pos *center)
{
  /* absolute center of the screen */
  int clip_center_x = w / 2;
  int clip_center_y = h / 2;

  /* print position of center tile */
  int center_x = clip_center_x - WIDTH / 2;
  int center_y = clip_center_y - HEIGHT / 2;

  /* XXX initialize global map ... 
   * normally this does not belong in here */
  static struct mmap glob_map = {0};
  if (!glob_map.w) {
    mmap_init(&glob_map, 10, 10, 0);
    mmap_set(&glob_map, 0, 0, 1);
    mmap_set(&glob_map, 0, 1, 1);
  }
  /* XXX */


  struct map_pos r_pos;
  r_pos = *center;
  map_round(&r_pos);

  struct screen_pos round;
  map2screen(&r_pos, &round);

  struct screen_pos screen;
  map2screen(center, &screen);


  int soft_scroll_screen_offset_x = screen.x - round.x;
  int soft_scroll_screen_offset_y = screen.y - round.y;

  /* setup clip rectangle */
  draw_clip_rect4(x + clip_center_x - w / 2, y + clip_center_y - h/2, w, h);

  center_x += soft_scroll_screen_offset_x;
  center_y += soft_scroll_screen_offset_y;


  int W = ceilf(w / (float)WIDTH) + 2;
  int H = ceilf(h / (HEIGHT * 0.75)) + 2;

  int off = 0;
  int o2 = 0;

  struct screen_pos local_mouse_pos = current_mouse_pos;
  local_mouse_pos.x -= x + clip_center_x + soft_scroll_screen_offset_x;
  local_mouse_pos.y -= y + clip_center_y + soft_scroll_screen_offset_y;

  struct cube_pos mouse_c_pos;
  screen2cube(&local_mouse_pos, &mouse_c_pos);
  cube_round(&mouse_c_pos);
  struct map_pos mouse_map_pos;
  cube2map(&mouse_c_pos, &mouse_map_pos);

  mouse_map_pos.x -= r_pos.x;
  mouse_map_pos.y -= r_pos.y;

  /* convert to integer for map-normalization */
  int mouse_map_x = mouse_map_pos.x;
  int mouse_map_y = mouse_map_pos.y;

  map_normalize_coordinates(&glob_map, &mouse_map_x, &mouse_map_y);


  int offset = ceilf(H/4.0);
  struct map_pos pos;
  for (pos.y = -(ceilf(H/2.0)); pos.y < ceilf(H/2.0); ++pos.y) {
    ++o2;
    off = o2 / 2;
    for (pos.x = -(ceilf(W/2.0)) + offset; pos.x < ceilf(W/2.0)+offset ; ++pos.x) {
      /* align x offset */
      pos.x -= off;

      map2screen(&pos, &screen);
      int map_x = pos.x - r_pos.x;
      int map_y = pos.y - r_pos.y;
      map_normalize_coordinates(&glob_map, &map_x, &map_y);
      int tile = mmap_get(&glob_map, map_x, map_y);
      draw_frame(x + center_x + screen.x, y + center_y + screen.y, tileset_get_frame_by_id(glob_tiles, tile));

      if (map_x == mouse_map_x && map_y == mouse_map_y) {
        draw_frame(x + center_x + screen.x, y + center_y + screen.y, tileset_get_frame_by_id(glob_tiles, 2));
      }

      /* reset x offset for for - loop */
      pos.x += off;
    }
  }

  draw_color(255,255,255,255);
  draw_clip_null();
}

static void draw(void *data)
{
  draw_color(0,0,0,255);
  clear_screen();


  struct map_pos real_center_pos;
  real_center_pos.x = center_pos.x + scroll_pos.x;
  real_center_pos.y = center_pos.y + scroll_pos.y;

 draw_map_test(0,0,SCREEN_WIDTH,SCREEN_HEIGHT, &real_center_pos);
}

static void key_up(int key, void *data)
{
  engine_quit();
}

static struct game_ctx ctx = {
  .screen_width = 200,
  .screen_height = 150,
  .screen_scale = 4,
  .game_init = init,
  .game_update = update,
  .game_draw = draw,
  .game_on_key_up = key_up,
  .game_on_mouse_motion = motion,
  .game_on_mouse_up = mouse_up,
  .game_on_mouse_down = mouse_down,
};

ENGINE_MAIN(&ctx);
