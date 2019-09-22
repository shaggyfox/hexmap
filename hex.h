struct screen_pos {
  int x;
  int y;
};

struct map_pos {
  float x;
  float y;
};

struct cube_pos {
  float x;
  float y;
  float z;
};

//#define WIDTH 20.0f
//#define HEIGHT 16.0f
#define WIDTH 12.0f
#define HEIGHT 8.0f

void map_to_offset(struct map_pos *m_pos, struct map_pos *o_pos);
void map_from_offset(struct map_pos *o_pos, struct map_pos *m_pos);
void cube_round(struct cube_pos *c_pos);
void cube2screen(struct cube_pos* c_pos, struct screen_pos *s_pos);
void screen2cube(struct screen_pos *s_pos, struct cube_pos *c_pos);
void cube2map(struct cube_pos *c_pos, struct map_pos *m_pos);
void map2cube(struct map_pos *m_pos, struct cube_pos *c_pos);

