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
  glob_tiles = tileset_load_from_file("../hextile.json");
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
  s_pos.x = mouse_x;
  s_pos.y = mouse_y;
  screen2cube(&s_pos, &c_pos);
  cube2map(&c_pos, &m_pos);
  clear_screen();
  draw_frame(s_pos.x, s_pos.y, tileset_get_frame_by_id(glob_tiles, 0));
  text_use_font(FONT_TINY);
  text_printf(0,0, "%03i,%03i", mouse_x, mouse_y);
  text_use_font(FONT_DEFAULT);
  text_printf(0,10, "%0.2f,%0.2f", m_pos.x, m_pos.y);
}

static void key_up(int key, void *data)
{
  engine_quit();
}

static struct game_ctx ctx = {
  .screen_width = 320,
  .screen_height = 240,
  .screen_scale = 3,
  .game_init = init,
  .game_update = update,
  .game_draw = draw,
  .game_on_key_up = key_up,
  .game_on_mouse_motion = motion,
  .game_on_mouse_up = NULL,
  .game_on_mouse_down = NULL,
};

ENGINE_MAIN(&ctx);
