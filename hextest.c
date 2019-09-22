#include "engine.h"
#include "hex.h"
#include "hextile.h"

int mouse_x = 0;
int mouse_y = 0;
struct tileset *glob_tiles = NULL;

static void init(void **data)
{
  draw_color(0,0,0,255);
  text_color(150,150,150,255);
  glob_tiles = tileset_load_raw_from_file("../hextile.png", WIDTH, HEIGHT);
}

static void update(void *data, float delta)
{
}

static void motion(int x, int y, void *data)
{
  mouse_x = x;
  mouse_y = y;
}

static void offset_map2screen(struct map_pos *map, struct screen_pos *screen)
{
  struct map_pos c_map;
  struct cube_pos cube;
  map_from_offset(&c_map, map);
  map2cube(&c_map, &cube);
  cube2screen(&cube, screen);
}

static void draw_map_tile(int x, int y, int w, int h, struct map_pos *pos, int id)
{
  int center_x = (w - WIDTH) / 2;
  int center_y = (h - HEIGHT) / 2;
  struct screen_pos screen;
  offset_map2screen(pos, &screen);
  draw_frame(x + center_x + screen.x, y + center_y + screen.y, tileset_get_frame_by_id(glob_tiles, id));
}

static void draw_map_test(int x, int y, int w, int h)
{
  /* find center of the screen */
  int center_x = (w - WIDTH) / 2;
  int center_y = (h - HEIGHT) / 2;

  struct map_pos pos;
  struct screen_pos screen;
  offset_map2screen(&pos, &screen);
  for (pos.y = -5; pos.y < 5; ++pos.y) {
    for (pos.x = -5; pos.x < 5; ++pos.x) {
      offset_map2screen(&pos, &screen);
      draw_frame(x + center_x + screen.x, y + center_y + screen.y, tileset_get_frame_by_id(glob_tiles, 0));
    }
  }
}

static void draw_map_test_mouse(int x, int y, int w, int h)
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

static void draw(void *data)
{
  draw_color(0,0,0,255);
  clear_screen();
  draw_map_test(0,0,SCREEN_WIDTH, SCREEN_HEIGHT);
  draw_map_test_mouse(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
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
  .game_on_mouse_up = NULL,
  .game_on_mouse_down = NULL,
};

ENGINE_MAIN(&ctx);
