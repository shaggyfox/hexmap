#include "engine.h"
#include "hex.h"
#include "hextile.h"

int mouse_is_down = 0;
struct map_pos center_pos = {0,0};
struct map_pos scroll_pos;
struct screen_pos mouse_pos = {0,0};
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

static void draw_map_tile(int x, int y, int w, int h, struct map_pos *pos, int id)
{
  int center_x = (w - WIDTH) / 2;
  int center_y = (h - HEIGHT) / 2;
  struct screen_pos screen;
  //offset_map2screen(pos, &screen);
  //draw_frame(x + center_x + screen.x, y + center_y + screen.y, tileset_get_frame_by_id(glob_tiles, id));
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
  map->data[y * map->w + x] = v;
}

int mmap_get(struct mmap* map, int x, int y)
{
  while (x < 0) {
    x += map->w;
  }
  while (y < 0) {
    y += map->h;
  }
  return map->data[map->w * (y % map->h) + (x % map->w)];
}

static void draw_map_test(int x, int y, int w, int h, struct map_pos *center)
{
  /* find center of the screen */
  int center_x = (w - WIDTH) / 2;
  int center_y = (h - HEIGHT) / 2;
  static struct mmap glob_map = {0};
  if (!glob_map.w) {
    mmap_init(&glob_map, 10, 10, 0);
    mmap_set(&glob_map, 0, 0, 1);
  }

  struct map_pos pos;
  struct map_pos r_pos;
  struct screen_pos round;
  struct screen_pos screen;
  r_pos = *center;
  map_round(&r_pos);
  map2screen(&r_pos, &round);
  map2screen(center, &screen);

  center_x += screen.x - round.x;
  center_y += screen.y - round.y;

  map_to_offset(&r_pos, &r_pos);

  for (pos.y = -5; pos.y < 5; ++pos.y) {
    for (pos.x = -5; pos.x < 5; ++pos.x) {
      offset_map2screen(&pos, &screen);
      draw_frame(x + center_x + screen.x, y + center_y + screen.y, tileset_get_frame_by_id(glob_tiles, mmap_get(&glob_map, pos.x - r_pos.x, pos.y - r_pos.y)));
    }
  }
}
#if 0
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
  s_pos.x = mouse_x - center_x;
  s_pos.y = mouse_y - center_y;

  screen2cube(&s_pos, &c_pos);
  cube2map(&c_pos, &plain_pos);
  cube_round(&c_pos);
  cube2map(&c_pos, &m_pos);
  map_to_offset(&m_pos, &m_pos);
  map_to_offset(&plain_pos, &plain_pos_off);
  offset_map2screen(&m_pos, &s_pos2);
  text_printf(0, 0, "%0.0f %0.0f %i %i", m_pos.x, m_pos.y, s_pos2.x - s_pos.x, s_pos2.y - s_pos.y);
  draw_map_tile(x, y, w, h, &m_pos, 1);

}
#endif

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
  .screen_width = 32*4,
  .screen_height = 24*4,
  .screen_scale = 8,
  .game_init = init,
  .game_update = update,
  .game_draw = draw,
  .game_on_key_up = key_up,
  .game_on_mouse_motion = motion,
  .game_on_mouse_up = mouse_up,
  .game_on_mouse_down = mouse_down,
};

ENGINE_MAIN(&ctx);
