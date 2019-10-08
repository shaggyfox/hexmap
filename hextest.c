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

void screen2offset_map(struct screen_pos *s_pos, struct map_pos *m_pos)
{
  struct cube_pos c_pos;
  screen2cube(s_pos, &c_pos);
  cube2map(&c_pos, m_pos);
  map_to_offset(m_pos, m_pos);
}

void screen2map(struct screen_pos *s_pos, struct map_pos *m_pos)
{
  struct cube_pos c_pos;
  screen2cube(s_pos, &c_pos);
  cube2map(&c_pos, m_pos);
}

void map2screen(struct map_pos *m_pos, struct screen_pos *s_pos)
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

static void offset_map2screen(struct map_pos *map, struct screen_pos *screen)
{
  struct map_pos c_map;
  struct cube_pos cube;
  offset_to_map(map, &c_map);
  map2cube(&c_map, &cube);
  cube2screen(&cube, screen);
}

static void offset_map_round(struct map_pos *map)
{
  offset_to_map(map, map);
  map_round(map);
  map_to_offset(map, map);
}

void draw_map_tile(int x, int y, int w, int h, struct map_pos *pos, int id)
{
  int center_x = (w - WIDTH) / 2;
  int center_y = (h - HEIGHT) / 2;
  struct screen_pos screen;
  map2screen(pos, &screen);
  draw_frame(x + center_x + screen.x, y + center_y + screen.y, tileset_get_frame_by_id(glob_tiles, id));
}

struct mmap {
  int w;
  int h;
  int *data;
};

void mmap_init(struct mmap *map, int w, int h, int v)
{
  map->w = w;
  map->h = h;
  map->data = malloc(w * h * sizeof(*map->data));
  for (int i = 0; i < w * h; ++i) {
    map->data[i] = v;
  }
}

void mmap_set(struct mmap* map, int x, int y, int v)
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

int mmap_get(struct mmap* map, int x, int y)
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
  int center_x = (w - WIDTH) / 2;
  int center_y = (h - HEIGHT) / 2;

  /* XXX initialize global map ... 
   * normally this does not belong in here */
  static struct mmap glob_map = {0};
  if (!glob_map.w) {
    mmap_init(&glob_map, 10, 10, 0);
    mmap_set(&glob_map, 0, 0, 1);
    mmap_set(&glob_map, 0, 1, 1);
  }

  struct map_pos pos;
  struct map_pos r_pos;
  struct screen_pos round;
  struct screen_pos screen;

  r_pos = *center;
  map_round(&r_pos);
  map2screen(&r_pos, &round);
  map2screen(center, &screen);

  /* setup clip rectangle */
  draw_clip_rect4(x + clip_center_x - w / 2, y + clip_center_y - h/2, w, h);

  center_x += screen.x - round.x;
  center_y += screen.y - round.y;
  int W = w / WIDTH * 1.5;
  int H = h / HEIGHT * 1.5;
  int o = roundf(H/4.0);

  int off = 0;
  int o2 = 0;


  struct screen_pos local_mouse_pos = current_mouse_pos;
  local_mouse_pos.x -= clip_center_x;
  local_mouse_pos.y -= clip_center_y;
  local_mouse_pos.x -= screen.x - round.x;
  local_mouse_pos.y -= screen.y - round.y;
  struct cube_pos mouse_c_pos;
  screen2cube(&local_mouse_pos, &mouse_c_pos);
  cube_round(&mouse_c_pos);
  struct map_pos mouse_map_pos;
  cube2map(&mouse_c_pos, &mouse_map_pos);
  mouse_map_pos.x -= r_pos.x;
  mouse_map_pos.y -= r_pos.y;
  int mouse_map_x = mouse_map_pos.x;
  int mouse_map_y = mouse_map_pos.y;

  map_normalize_coordinates(&glob_map, &mouse_map_x, &mouse_map_y);
  printf("mouse_map= %f, %f\n", mouse_map_pos.x, mouse_map_pos.y);

  for (pos.y = -H/2; pos.y < H/2; ++pos.y) {
    ++o2;
    if (o2 == 2) {
      o2 = 0;
      off += 1;
    }
    for (pos.x = -4; pos.x < W; ++pos.x) {
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

  draw_clip_null();
}

void draw_map_test_mouse(int x, int y, int w, int h)
{
  int center_x = w / 2;
  int center_y = h / 2;
  struct screen_pos s_pos;
  struct screen_pos s_pos2;
  struct cube_pos c_pos;
  struct map_pos m_pos;
  struct map_pos plain_pos;
  struct map_pos plain_pos_off;
  s_pos.x = mouse_pos.x - center_x;
  s_pos.y = mouse_pos.y - center_y;

  screen2cube(&s_pos, &c_pos);
  cube2map(&c_pos, &plain_pos);
  cube_round(&c_pos);
  cube2map(&c_pos, &m_pos);
  draw_map_tile(x, y, w, h, &m_pos, 1);

}

static void draw(void *data)
{
  draw_color(0,0,0,255);
  clear_screen();


  //struct screen_pos pos;
  //struct screen_pos round_pos;
  //map2screen(&new_center_pos, &pos);
  //draw_frame(pos.x +SCREEN_WIDTH / 2,  pos.y + SCREEN_HEIGHT / 2, tileset_get_frame_by_id(glob_tiles, 0));
  //printf("%d %d\n", pos.x, pos.y);

  struct map_pos real_center_pos;
  real_center_pos.x = center_pos.x + scroll_pos.x;
  real_center_pos.y = center_pos.y + scroll_pos.y;

  draw_map_test(0,0,SCREEN_WIDTH, SCREEN_HEIGHT, &real_center_pos);
  //draw_map_test_mouse(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
#if 0
  struct screen_pos s_pos;
  struct cube_pos c_pos;
  struct map_pos m_pos;
  s_pos.x = mouse_x - WIDTH / 2;// - HEIGHT; //WIDTH / 3;
  s_pos.y = mouse_y - HEIGHT / 2; // + WIDTH/8;// - HEIGHT / 4;
  screen2cube(&s_pos, &c_pos);
  cube_round(&c_pos);
  cube2map(&c_pos, &m_pos);
  draw_color(0,0,0,255);
  clear_screen();
  //text_use_font(FONT_TINY);
  text_printf(0,0, "%03i %03i", mouse_x, mouse_y);
  text_printf(0,10, "%0.02f %0.02f %0.02f", c_pos.x, c_pos.y, c_pos.z);
  map_to_offset(&m_pos);
  text_printf(0,20, "%0.2f %0.2f", m_pos.x, m_pos.y);
  //map2cube(&m_pos, &c_pos);
  cube2screen(&c_pos, &s_pos);
  draw_frame(s_pos.x, s_pos.y, tileset_get_frame_by_id(glob_tiles, 0));
  draw_color(255,255,255,255);
  draw_point(mouse_x, mouse_y);
#endif
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
