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

#define WIDTH 12.0f
#define HEIGHT 8.0f

void cube2screen(struct cube_pos* c_pos, struct screen_pos *s_pos);
void screen2cube(struct screen_pos *s_pos, struct cube_pos *c_pos);
void cube2map(struct cube_pos *c_pos, struct map_pos *m_pos);
void map2cube(struct map_pos *m_pos, struct cube_pos *c_pos);

