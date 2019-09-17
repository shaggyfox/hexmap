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
  glob_tiles = tileset_load_raw_from_file("../hextile.png", 20, 16);
}

static void update(void *data, float delta)
{
}

static void motion(int x, int y, void *data)
{
  mouse_x = x;
  mouse_y = y;
}

static void draw(void *data)
{
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
